#import "EJConvert.h"

NSString *JSValueToNSString( JSContextRef ctx, JSValueRef v ) {
	JSStringRef jsString = JSValueToStringCopy( ctx, v, NULL );
	if( !jsString ) return nil;
	
	NSString *string = (NSString *)JSStringCopyCFString( kCFAllocatorDefault, jsString );
	[string autorelease];
	JSStringRelease( jsString );
	
	return string;
}

JSValueRef NSStringToJSValue( JSContextRef ctx, NSString *string ) {
	JSStringRef jstr = JSStringCreateWithCFString((CFStringRef)string);
	JSValueRef ret = JSValueMakeString(ctx, jstr);
	JSStringRelease(jstr);
	return ret;
}



double JSValueToNumberFast(JSContextRef ctx, JSValueRef v) {
	#if __LP64__ // arm64 version
		union {
			int64_t asInt64;
			double asDouble;
			struct { int32_t asInt; int32_t tag; } asBits;
		} taggedValue = { .asInt64 = (int64_t)v };
		
		#define DoubleEncodeOffset 0x1000000000000ll
		#define TagTypeNumber 0xffff0000
		#define ValueTrue 0x7
		
		if( (taggedValue.asBits.tag & TagTypeNumber) == TagTypeNumber ) {
			return taggedValue.asBits.asInt;
		}
		else if( taggedValue.asBits.tag & TagTypeNumber ) {
			taggedValue.asInt64 -= DoubleEncodeOffset;
			return taggedValue.asDouble;
		}
		else if( taggedValue.asBits.asInt == ValueTrue ) {
			return 1.0;
		}
		else {
			return 0; // false, undefined, null, object
		}
	#else // armv7 version
		return JSValueToNumber(ctx, v, NULL);
	#endif
}

void JSValueUnprotectSafe( JSContextRef ctx, JSValueRef v ) {
	if( ctx && v ) {
		JSValueUnprotect(ctx, v);
	}
}

JSValueRef NSObjectToJSValue( JSContextRef ctx, NSObject *obj ) {
	JSValueRef ret = NULL;
	
	if( [obj isKindOfClass:NSString.class] ) {
		ret = NSStringToJSValue(ctx, (NSString *)obj);
	}
	
	else if( [obj isKindOfClass:NSNumber.class] ) {
		NSNumber *number = (NSNumber *)obj;
		if( strcmp(number.objCType, @encode(BOOL)) == 0 ) {
			ret = JSValueMakeBoolean(ctx, number.boolValue);
		}
		else {
			ret = JSValueMakeNumber(ctx, number.doubleValue);
		}
	}
	
	else if( [obj isKindOfClass:NSDate.class] ) {
		NSDate *date = (NSDate *)obj;
		JSValueRef timestamp = JSValueMakeNumber(ctx, date.timeIntervalSince1970 * 1000.0);
		ret = JSObjectMakeDate(ctx, 1, &timestamp, NULL);
	}
	
	else if( [obj isKindOfClass:NSArray.class] ) {
		NSArray *array = (NSArray *)obj;
		JSValueRef *args = malloc(array.count * sizeof(JSValueRef));
		for( int i = 0; i < array.count; i++ ) {
			args[i] = NSObjectToJSValue(ctx, array[i] );
		}
		ret = JSObjectMakeArray(ctx, array.count, args, NULL);
		free(args);
	}
	
	else if( [obj isKindOfClass:NSDictionary.class] ) {
		NSDictionary *dict = (NSDictionary *)obj;
		ret = JSObjectMake(ctx, NULL, NULL);
		for( NSString *key in dict ) {
			JSStringRef jsKey = JSStringCreateWithUTF8CString(key.UTF8String);
			JSValueRef value = NSObjectToJSValue(ctx, dict[key]);
			JSObjectSetProperty(ctx, (JSObjectRef)ret, jsKey, value, NULL, NULL);
			JSStringRelease(jsKey);
		}
	}
	
	return ret ? ret : JSValueMakeNull(ctx);
}

NSObject *JSValueToNSObject( JSContextRef ctx, JSValueRef value ) {
	JSType type = JSValueGetType(ctx, value);
	
	switch( type ) {
		case kJSTypeString: return JSValueToNSString(ctx, value);
		case kJSTypeBoolean: return [NSNumber numberWithBool:JSValueToBoolean(ctx, value)];
		case kJSTypeNumber: return [NSNumber numberWithDouble:JSValueToNumberFast(ctx, value)];
		case kJSTypeNull: return nil;
		case kJSTypeUndefined: return nil;
		case kJSTypeObject: break;
	}
	
	if( type == kJSTypeObject ) {
		JSObjectRef jsObj = (JSObjectRef)value;
		
		JSStringRef arrayName = JSStringCreateWithUTF8CString("Array");
		JSObjectRef arrayConstructor = (JSObjectRef)JSObjectGetProperty(ctx, JSContextGetGlobalObject(ctx), arrayName, NULL);
		JSStringRelease(arrayName);
			
		if( JSValueIsInstanceOfConstructor(ctx, jsObj, arrayConstructor, NULL) ) {
			JSStringRef lengthName = JSStringCreateWithUTF8CString("length");
			int count = JSValueToNumberFast(ctx, JSObjectGetProperty(ctx, jsObj, lengthName, NULL));
			JSStringRelease(lengthName);
			
			NSMutableArray *array = [NSMutableArray arrayWithCapacity:count];
			for( int i = 0; i < count; i++ ) {
				NSObject *obj = JSValueToNSObject(ctx, JSObjectGetPropertyAtIndex(ctx, jsObj, i, NULL));
				[array addObject:(obj ? obj : NSNull.null)];
			}
			return array;
		}
		else {
			JSPropertyNameArrayRef properties = JSObjectCopyPropertyNames(ctx, jsObj);
			size_t count = JSPropertyNameArrayGetCount(properties);
			
			NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithCapacity:count];
			for( size_t i = 0; i < count; i++ ) {
				JSStringRef jsName = JSPropertyNameArrayGetNameAtIndex(properties, i);
				NSObject *obj = JSValueToNSObject(ctx, JSObjectGetProperty(ctx, jsObj, jsName, NULL));
				
				NSString *name = (NSString *)JSStringCopyCFString( kCFAllocatorDefault, jsName );
				dict[name] = obj ? obj : NSNull.null;
				[name release];
			}
			
			JSPropertyNameArrayRelease(properties);
			return dict;
		}
	}
	
	return nil;
}

