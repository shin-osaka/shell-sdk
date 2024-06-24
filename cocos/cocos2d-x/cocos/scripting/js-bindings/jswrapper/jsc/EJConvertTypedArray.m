#import "EJConvertTypedArray.h"
#import "EJConvert.h"

#if defined __ARM_NEON__
#include <arm_neon.h>
#endif














const static char *ConstructorNames[] = {
	[kEJJSTypedArrayTypeNone] = NULL,
	[kEJJSTypedArrayTypeInt8Array] = "Int8Array",
	[kEJJSTypedArrayTypeInt16Array] = "Int16Array",
	[kEJJSTypedArrayTypeInt32Array] = "Int32Array",
	[kEJJSTypedArrayTypeUint8Array] = "Uint8Array",
	[kEJJSTypedArrayTypeUint8ClampedArray] = "Uint8ClampedArray",
	[kEJJSTypedArrayTypeUint16Array] = "Uint16Array",
	[kEJJSTypedArrayTypeUint32Array] = "Uint32Array",
	[kEJJSTypedArrayTypeFloat32Array] = "Float32Array",
	[kEJJSTypedArrayTypeFloat64Array] = "Float64Array",
	[kEJJSTypedArrayTypeArrayBuffer] = "ArrayBuffer"
};



const static int CopyInChunksThreshold = 32;



const static int CopyChunkSize = 0x4000;






static inline int32_t GetInt32(JSContextRef ctx, JSValueRef v) {
	#if __LP64__
		return (int32_t)(0x00000000ffffffffll & (uint64_t)v);
	#else
		return JSValueToNumber(ctx, v, NULL);
	#endif
}



static inline JSValueRef MakeInt32(JSContextRef ctx, int32_t number) {
	#if __LP64__
		return (0xffff000000000000ll | (uint64_t)number);
	#else
		return JSValueMakeNumber(ctx, number);
	#endif
}



static JSValueRef GetPropertyNamed(JSContextRef ctx, JSObjectRef object, const char *name) {
	JSStringRef jsPropertyName = JSStringCreateWithUTF8CString(name);
	JSValueRef value = JSObjectGetProperty(ctx, object, jsPropertyName, NULL);
	JSStringRelease(jsPropertyName);
	return value;
}



static JSObjectRef GetConstructor(JSContextRef ctx, EJJSTypedArrayType type) {
	if( type <= kEJJSTypedArrayTypeNone || type > kEJJSTypedArrayTypeArrayBuffer ) {
		return NULL;
	}
	
	const char *constructorName = ConstructorNames[type];
	JSObjectRef global = JSContextGetGlobalObject(ctx);
	return (JSObjectRef)GetPropertyNamed(ctx, global, constructorName);
}



static JSObjectRef GetView(JSContextRef ctx, JSObjectRef object, EJJSTypedArrayType type, size_t count) {
	EJJSTypedArrayType currentType = EJJSObjectGetTypedArrayType(ctx, object);
	if( currentType == kEJJSTypedArrayTypeNone ) {
		return NULL;
	}
	else if( currentType == type ) {
		return object;
	}
	
	JSValueRef args[3];
	if( currentType == kEJJSTypedArrayTypeArrayBuffer ) {
		args[0] = object;
		args[1] = MakeInt32(ctx, 0);
		args[2] = MakeInt32(ctx, (int)count);
	}
	else {
		args[0] = GetPropertyNamed(ctx, object, "buffer");
		args[1] = GetPropertyNamed(ctx, object, "byteOffset");
		args[2] = MakeInt32(ctx, (int)count);
	}
	JSObjectRef constructor = GetConstructor(ctx, type);
	return JSObjectCallAsConstructor(ctx, constructor, 3, args, NULL);
}



typedef struct {
	int32_t *currentDataPtr;
	JSObjectRef jsGetCallback;
	JSObjectRef jsGetCallbackApply;
} AppendDataCallbackState;

static JSValueRef AppendDataCallback(
	JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
	size_t argc, const JSValueRef argv[], JSValueRef* exception
) {
	AppendDataCallbackState *state = JSObjectGetPrivate(thisObject);
	int32_t *dst = state->currentDataPtr;
	int remainderStart = 0;
	
	
	#if __LP64__ && defined __ARM_NEON__
	
		int argPacks4 = (int)argc/4;
		int32_t *src = (int32_t*)argv;
	
		for(int i = 0; i < argPacks4; i++ ) {
			const int32x4x2_t lanes32 = vld2q_s32(src);
			vst1q_s32(dst, lanes32.val[0]);
			src += 8;
			dst += 4;
		}
		remainderStart = argPacks4 * 4;
	#endif
	
	for( int i = remainderStart; i < argc; i++ ) {
		*(dst++) = GetInt32(ctx, argv[i]);
	}
	
	state->currentDataPtr += argc;
	return MakeInt32(ctx, (int)argc);
}



static void FinalizeAppendDataCallbackState(JSObjectRef object) {
	AppendDataCallbackState *state = JSObjectGetPrivate(object);
	free(state);
}

static JSObjectRef CreateAppendDataCallbackState(JSContextRef ctx) {
	JSObjectRef jsAppendDataCallback = JSObjectMakeFunctionWithCallback(ctx, NULL, AppendDataCallback);
	JSValueProtect(ctx, jsAppendDataCallback);
	
	AppendDataCallbackState *state = malloc(sizeof(AppendDataCallbackState));
	state->currentDataPtr = NULL;
	state->jsGetCallback = jsAppendDataCallback;
	state->jsGetCallbackApply = (JSObjectRef)GetPropertyNamed(ctx, jsAppendDataCallback, "apply");
	
	
	JSClassDefinition internalStateClassDef = kJSClassDefinitionEmpty;
	internalStateClassDef.finalize = FinalizeAppendDataCallbackState;
	
	JSClassRef internalStateClass = JSClassCreate(&internalStateClassDef);
	JSObjectRef internalStateObject = JSObjectMake(ctx, internalStateClass, state);
	JSClassRelease(internalStateClass);
	return internalStateObject;
}




void EJJSContextPrepareTypedArrayAPI(JSContextRef ctx) {
	JSPropertyAttributes attributes =
		kJSPropertyAttributeReadOnly |
		kJSPropertyAttributeDontEnum |
		kJSPropertyAttributeDontDelete;
	
	JSStringRef jsTypeName = JSStringCreateWithUTF8CString("__ejTypedArrayType");
	
	for( int type = kEJJSTypedArrayTypeInt8Array; type <= kEJJSTypedArrayTypeArrayBuffer; type++ ) {
		JSObjectRef jsConstructor = GetConstructor(ctx, type);
		JSObjectRef jsPrototype = (JSObjectRef)GetPropertyNamed(ctx, jsConstructor, "prototype");
		
		JSValueRef jsType = MakeInt32(ctx, type);
		JSObjectSetProperty(ctx, jsPrototype, jsTypeName, jsType, attributes, NULL);
	}
	
	JSStringRelease(jsTypeName);
	
	
	JSObjectRef jsCallbackStateObject = CreateAppendDataCallbackState(ctx);

	JSStringRef jsInternalStateName = JSStringCreateWithUTF8CString("__ejTypedArrayState");
	JSObjectRef global = JSContextGetGlobalObject(ctx);
	JSObjectSetProperty(ctx, global, jsInternalStateName, jsCallbackStateObject, attributes, NULL);
	JSStringRelease(jsInternalStateName);
}


EJJSTypedArrayType EJJSObjectGetTypedArrayType(JSContextRef ctx, JSObjectRef object) {
	JSValueRef jsType = GetPropertyNamed(ctx, object, "__ejTypedArrayType");
	return jsType ? GetInt32(ctx, jsType) : kEJJSTypedArrayTypeNone;
}

JSObjectRef EJJSObjectMakeTypedArray(JSContextRef ctx, EJJSTypedArrayType arrayType, size_t numElements) {
	JSObjectRef jsConstructor = GetConstructor(ctx, arrayType);
	if( !jsConstructor ) {
		return NULL;
	}
	
	JSValueRef jsNumElements = MakeInt32(ctx, (int)numElements);
	return JSObjectCallAsConstructor(ctx, jsConstructor, 1, (JSValueRef[]){jsNumElements}, NULL);
}


NSMutableData *EJJSObjectGetTypedArrayData(JSContextRef ctx, JSObjectRef object) {
	size_t length = GetInt32(ctx, GetPropertyNamed(ctx, object, "byteLength"));
	if( !length ) {
		return NULL;
	}
	
	size_t int32Count = length / 4;
	size_t uint8Count = length % 4;
	
	if( length < CopyInChunksThreshold ) {
		int32Count = 0;
		uint8Count = length;
	}
	
	NSMutableData *data = [NSMutableData dataWithLength:length];
	
	if( int32Count ) {
		JSObjectRef int32View = GetView(ctx, object, kEJJSTypedArrayTypeInt32Array, int32Count);
		if( !int32View ) {
			return NULL;
		}
		
		JSObjectRef jsState = (JSObjectRef)GetPropertyNamed(ctx, JSContextGetGlobalObject(ctx), "__ejTypedArrayState");
		AppendDataCallbackState *state = JSObjectGetPrivate(jsState);
		state->currentDataPtr = data.mutableBytes;
		
		
		if( int32Count < CopyChunkSize ) {
			JSValueRef getArgs[] = {jsState, int32View};
			JSObjectCallAsFunction(ctx, state->jsGetCallbackApply, state->jsGetCallback, 2, getArgs, NULL);
		}
		else {
			JSObjectRef subarrayFunc = (JSObjectRef)GetPropertyNamed(ctx, int32View, "subarray");
			
			for( int i = 0; i < int32Count; i+= CopyChunkSize) {
				JSValueRef subarrayArgs[] = {MakeInt32(ctx, i), MakeInt32(ctx, i + CopyChunkSize)};
				JSObjectRef jsSubarray = (JSObjectRef)JSObjectCallAsFunction(ctx, subarrayFunc, int32View, 2, subarrayArgs, NULL);
				
				JSValueRef getArgs[] = {jsState, jsSubarray};
				JSObjectCallAsFunction(ctx, state->jsGetCallbackApply, state->jsGetCallback, 2, getArgs, NULL);
			}
		}
	}
	
	if( uint8Count ) {
		uint8_t *values8 = data.mutableBytes;
		JSObjectRef uint8View = GetView(ctx, object, kEJJSTypedArrayTypeUint8Array, length);
		for( int i = 0; i < uint8Count; i++ ) {
			int index = (int)int32Count * 4 + i;
			values8[index] = GetInt32(ctx, JSObjectGetPropertyAtIndex(ctx, uint8View, index, NULL));
		}
	}
	
	return data;
}


void EJJSObjectSetTypedArrayData(JSContextRef ctx, JSObjectRef object, NSData *data) {
	size_t int32Count = data.length / 4;
	size_t uint8Count = data.length % 4;
	
	if( int32Count ) {
		JSObjectRef int32View = GetView(ctx, object, kEJJSTypedArrayTypeInt32Array, int32Count);
		if( !int32View ) {
			return;
		}
		
		JSValueRef *jsValues = malloc(int32Count  * sizeof(JSValueRef));
		
		const int32_t *values32 = data.bytes;
		for(int i = 0; i < int32Count; i++) {
			jsValues[i] = MakeInt32(ctx, values32[i]);
		}
		JSObjectRef jsArray = JSObjectMakeArray(ctx, int32Count, jsValues, NULL);
		
		free(jsValues);
		
		JSObjectRef setFunction = (JSObjectRef)GetPropertyNamed(ctx, int32View, "set");
		JSObjectCallAsFunction(ctx, setFunction, int32View, 1, (JSValueRef[]){jsArray}, NULL);
	}
	
	if( uint8Count ) {
		const uint8_t *values8 = data.bytes;
		JSObjectRef uint8View = GetView(ctx, object, kEJJSTypedArrayTypeUint8Array, data.length);
		for( int i = 0; i < uint8Count; i++ ) {
			int index = (int)int32Count * 4 + i;
			JSObjectSetPropertyAtIndex(ctx, uint8View, index, MakeInt32(ctx, values8[index]), NULL);
		}
	}
}
