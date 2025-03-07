#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>


#ifdef __cplusplus
extern "C" {
#endif

NSString *JSValueToNSString( JSContextRef ctx, JSValueRef v );
JSValueRef NSStringToJSValue( JSContextRef ctx, NSString *string );
double JSValueToNumberFast( JSContextRef ctx, JSValueRef v );
void JSValueUnprotectSafe( JSContextRef ctx, JSValueRef v );
JSValueRef NSObjectToJSValue( JSContextRef ctx, NSObject *obj );
NSObject *JSValueToNSObject( JSContextRef ctx, JSValueRef value );

static inline void *JSValueGetPrivate(JSValueRef v) {

	#if __LP64__
		return !((int64_t)v & 0xffff000000000002ll)
			? JSObjectGetPrivate((JSObjectRef)v)
			: NULL;
	#else
		return JSObjectGetPrivate((JSObjectRef)v);
	#endif
}

#ifdef __cplusplus
}
#endif
