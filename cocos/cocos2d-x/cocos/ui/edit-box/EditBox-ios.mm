/****************************************************************************
 Copyright (c) 2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#include "EditBox.h"
#include "platform/CCApplication.h"
#include "platform/ios/CCEAGLView-ios.h"
#include "cocos/scripting/js-bindings/jswrapper/SeApi.h"
#include "cocos/scripting/js-bindings/manual/jsb_global.h"

#import <UIKit/UITextField.h>
#import <UIKit/UITextView.h>

#define TEXT_LINE_HEIGHT  40
#define TEXT_VIEW_MAX_LINE_SHOWN    3
#define BUTTON_HIGHT    (TEXT_LINE_HEIGHT - 2)
#define BUTTON_WIDTH    60

#define TO_TEXT_VIEW(textinput)   ((UITextView*)textinput)
#define TO_TEXT_FIELD(textinput)  ((UITextField*)textinput)

#define IsStrEmpty(_ref)(( [(_ref) isKindOfClass:[NSNull class]]||(_ref) == nil) || ([(_ref) isEqual:[NSNull null]]) ||([(_ref)isEqualToString:@""]) || ([(_ref) isEqualToString:@""]) )

/*************************************************************************
 Inner class declarations.
 ************************************************************************/


@interface ButtonHandler : NSObject
-(IBAction) buttonTapped:(UIButton *)button;
@end

@interface KeyboardEventHandler : NSObject
-(void)keyboardWillShow: (NSNotification*) notification;
-(void)keyboardWillHide: (NSNotification*) notification;
-(void)keyboardWillChangeFrame: (NSNotification *) notification;
@end

@interface CommaTool : NSObject
+(NSString*)changeNumberCommaFormatter:(NSString*)str;
+(NSString*)changeNumberNormalFormatter:(NSString*)str;
@end

@interface TextFieldDelegate : NSObject<UITextFieldDelegate>
- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string;
- (void)textFieldDidChange:(UITextField *)textField;
- (BOOL)textFieldShouldReturn:(UITextField *)textField;
- (void)textFieldDidBeginEditing:(UITextField *)textField;
@end

@interface TextViewDelegate : NSObject<UITextViewDelegate>
- (BOOL) textView:(UITextView *)textView shouldChangeTextInRange:(NSRange)range replacementText:(NSString *)text;
- (void) textViewDidChange:(UITextView *)textView;
@end

/*************************************************************************
 Global variables and functions.
 ************************************************************************/


namespace
{
    struct SubView
    {
        int y = 0;
        int height = 0;
    };
    SubView sub = SubView();
    bool g_isMultiline = false;
    bool g_confirmHold = false;
    int g_maxLength = INT_MAX;
    int oldKeyboardHeight = 0;
    KeyboardEventHandler* g_keyboardHandler = nil;

    static UIColor* g_darkGreen = [UIColor colorWithRed:31/255.0 green:160/255.0 blue:20/255.0 alpha:0.8];

    UITextField* g_textField = nil;
    TextFieldDelegate* g_textFieldDelegate = nil;
    UIButton* g_textFieldConfirmButton = nil;
    ButtonHandler* g_textFieldConfirmButtonHandler = nil;
    
    UITextView* g_textView = nil;
    TextViewDelegate* g_textViewDelegate = nil;
    UIButton* g_textViewConfirmButton = nil;
    ButtonHandler* g_textViewConfirmButtonHander = nil;
    
    UIView *textBgView = nil;

    UIView* getCurrentView()
    {
        if (g_isMultiline)
            return g_textView;
        else
            return g_textField;
    }
    
    NSString* getCurrentText()
    {
        if (g_isMultiline)
            return g_textView.text;
        else
            return g_textField.text;
    }
    
    void setText(NSString* text)
    {
        if (g_isMultiline)
            g_textView.text = text;
        else
            g_textField.text = text;
    }
    
    se::Value textInputCallback;
    
    void getTextInputCallback()
    {
        if (! textInputCallback.isUndefined())
            return;
        
        auto global = se::ScriptEngine::getInstance()->getGlobalObject();
        se::Value jsbVal;
        if (global->getProperty("jsb", &jsbVal) && jsbVal.isObject())
        {
            jsbVal.toObject()->getProperty("onTextInput", &textInputCallback);
            se::ScriptEngine::getInstance()->addBeforeCleanupHook([](){
                textInputCallback.setUndefined();
            });
        }
    }
    
    void callJSFunc(const std::string& type, const std::string& text)
    {
        getTextInputCallback();
        
        se::AutoHandleScope scope;
        se::ValueArray args;
        args.push_back(se::Value(type));
        args.push_back(se::Value(text));
        textInputCallback.toObject()->call(args, nullptr);
    }
    
    int getTextInputHeight()
    {
        if (g_isMultiline)
            return 20;
        else
            return 20;
    }
    
    void createButton(UIButton** button, ButtonHandler** buttonHandler, const CGRect& viewRect, const std::string& title)
    {
        ButtonHandler *btnHandler = [[ButtonHandler alloc] init];
        UIButton* btn = [UIButton buttonWithType:UIButtonTypeRoundedRect];
        btn.hidden = YES;
        [btn addTarget:btnHandler action:@selector(buttonTapped:)
           forControlEvents:UIControlEventTouchUpInside];
        btn.frame = CGRectMake(0, 0, BUTTON_WIDTH, BUTTON_HIGHT);
        btn.backgroundColor = g_darkGreen;
        [btn setTitle: [NSString stringWithUTF8String:title.c_str()]
                forState:UIControlStateNormal];
        [btn setTitleColor: [UIColor whiteColor]
             forState:UIControlStateNormal];
        
        *button = btn;
        *buttonHandler = btnHandler;
    }
    
    void setTexFiledKeyboardType(UITextField* textField, const std::string& inputType)
    {
        if (0 == inputType.compare("password"))
        {
            textField.secureTextEntry = TRUE;
            textField.keyboardType = UIKeyboardTypeDefault;
        }
        else
        {
            textField.secureTextEntry = FALSE;
            if (0 == inputType.compare("email"))
                textField.keyboardType = UIKeyboardTypeEmailAddress;
            else if (0 == inputType.compare("number"))
                textField.keyboardType = UIKeyboardTypeASCIICapableNumberPad;
            else if (0 == inputType.compare("url"))
                textField.keyboardType = UIKeyboardTypeURL;
            else if (0 == inputType.compare("text"))
                textField.keyboardType = UIKeyboardTypeDefault;
            else if (0 == inputType.compare("phone"))
                textField.keyboardType = UIKeyboardTypePhonePad;
            else if (0 == inputType.compare("decimal"))
                textField.keyboardType = UIKeyboardTypeDecimalPad;
        }
    }
    
    void setTextFieldReturnType(UITextField* textField, const std::string& returnType)
    {
        if (0 == returnType.compare("done"))
            textField.returnKeyType = UIReturnKeyDone;
        else if (0 == returnType.compare("next"))
            textField.returnKeyType = UIReturnKeyNext;
        else if (0 == returnType.compare("search"))
            textField.returnKeyType = UIReturnKeySearch;
        else if (0 == returnType.compare("go"))
            textField.returnKeyType = UIReturnKeyGo;
        else if (0 == returnType.compare("send"))
            textField.returnKeyType = UIReturnKeySend;
    }

    NSString* getConfirmButtonTitle(const std::string& returnType)
    {
        NSString* titleKey = [NSString stringWithUTF8String: returnType.c_str()];
        return NSLocalizedString(titleKey, nil); // get i18n string to be the title
    }
    
    void initTextField(const CGRect& rect, const cocos2d::EditBox::ShowInfo& showInfo)
    {
        if (! g_textField)
        {
            g_textField = [[UITextField alloc] initWithFrame:rect];
            [g_textField setBorderStyle:UITextBorderStyleLine];
            g_textField.backgroundColor = [UIColor whiteColor];
            
            g_textFieldDelegate = [[TextFieldDelegate alloc] init];
            g_textField.delegate = g_textFieldDelegate;
            
            g_textField.rightViewMode = UITextFieldViewModeAlways;
            [g_textField addTarget:g_textFieldDelegate action:@selector(textFieldDidChange:) forControlEvents:UIControlEventEditingChanged];
        }

        g_textField.frame = rect;
        setTextFieldReturnType(g_textField, showInfo.confirmType);
        setTexFiledKeyboardType(g_textField, showInfo.inputType);
        g_textField.text = [NSString stringWithUTF8String: showInfo.defaultValue.c_str()];
        [g_textFieldConfirmButton setTitle:getConfirmButtonTitle(showInfo.confirmType) forState:UIControlStateNormal];
    }
    
    void initTextView(const CGRect& viewRect, const CGRect& btnRect, const cocos2d::EditBox::ShowInfo& showInfo)
    {
        if (!g_textView)
        {
            g_textView = [[UITextView alloc] initWithFrame:btnRect];
            
            g_textViewDelegate = [[TextViewDelegate alloc] init];
            g_textView.delegate = g_textViewDelegate;
            
            
            textBgView = [[UIView alloc] initWithFrame:btnRect];
            textBgView.backgroundColor = [UIColor whiteColor];
            textBgView.hidden = YES;
        }
        
        g_textView.frame = btnRect;
        [g_textView setReturnKeyType:UIReturnKeyDone];
        g_textView.text = [NSString stringWithUTF8String: showInfo.defaultValue.c_str()];
        [g_textViewConfirmButton setTitle:getConfirmButtonTitle(showInfo.confirmType) forState:UIControlStateNormal];
    }
    
    void addTextInput(const cocos2d::EditBox::ShowInfo& showInfo)
    {
        UIView* view = (UIView*)cocos2d::Application::getInstance()->getView();
        CGRect viewRect = view.frame;
        int height = getTextInputHeight();
        CGRect rect = CGRectMake(viewRect.origin.x,
                                 viewRect.size.height - height,
                                 viewRect.size.width,
                                 height);
        if (showInfo.isMultiline)
            initTextView(viewRect, rect, showInfo);
        else
            initTextField(rect, showInfo);
        
        UIScreen *screen = [UIScreen mainScreen];
        
        sub.height = showInfo.height;
        sub.y = viewRect.size.height - showInfo.y * viewRect.size.width / screen.nativeBounds.size.height;
        
        UIView* textInput = getCurrentView();
        [view addSubview:textBgView];
        [view addSubview:textInput];
        [textInput performSelector:@selector(becomeFirstResponder) withObject:nil afterDelay:0];
    }
    
    void addKeyboardEventLisnters()
    {
        if (!g_keyboardHandler)
            g_keyboardHandler = [[KeyboardEventHandler alloc] init];
        
        [[NSNotificationCenter defaultCenter] addObserver:g_keyboardHandler
                                                 selector:@selector(keyboardWillShow:)
                                                     name:UIKeyboardWillShowNotification
                                                   object:nil];
        
        [[NSNotificationCenter defaultCenter] addObserver:g_keyboardHandler
                                                 selector:@selector(keyboardWillHide:)
                                                     name:UIKeyboardWillHideNotification
                                                   object:nil];
        
        [[NSNotificationCenter defaultCenter] addObserver:g_keyboardHandler
                                                 selector:@selector(keyboardWillChangeFrame:)
                                                     name:UIKeyboardWillChangeFrameNotification
                                                   object:nil];
    }
    
    void removeKeyboardEventLisnters()
    {
        if (!g_keyboardHandler)
            return;
        
        [[NSNotificationCenter defaultCenter] removeObserver:g_keyboardHandler];
    }
}

/*************************************************************************
 Class implementations.
 ************************************************************************/


@implementation KeyboardEventHandler
-(void)keyboardWillShow: (NSNotification*) notification
{
    UIView* textView = getCurrentView();
    if (!textView)
        return;

    NSDictionary* keyboardInfo = [notification userInfo];
    NSValue* keyboardFrame = [keyboardInfo objectForKey:UIKeyboardFrameEndUserInfoKey];
    CGSize kbSize = [keyboardFrame CGRectValue].size;

    int textHeight = getTextInputHeight();
    
    if ([textView isKindOfClass:[UITextView class]]) {
        float h = UIApplication.sharedApplication.keyWindow.frame.size.height;
        textView.frame = CGRectMake(0,0,0,0);
        
        if (@available(iOS 11.0, *)) {
            UIEdgeInsets edges = [UIApplication sharedApplication].keyWindow.safeAreaInsets;
            textBgView.frame = CGRectMake(0, 0, kbSize.width, h - kbSize.height);
            textBgView.hidden = NO;
            
            textView.frame = CGRectMake(0,0,0,0);
        }
        return;
    }
    
    int offset_x = 0;
    if (@available(iOS 11.0, *)) {
        UIEdgeInsets edges = [UIApplication sharedApplication].keyWindow.safeAreaInsets;
        if (edges.right != 0) {
            offset_x = edges.right - 15;
        }
    }

    UIView* screenView = (UIView*)cocos2d::Application::getInstance()->getView();
    screenView.frame = CGRectMake(0, 0, screenView.frame.size.width, screenView.frame.size.height);
    CGRect screenRect = screenView.frame;

    oldKeyboardHeight = kbSize.height;
    
    int offset_y = screenRect.size.height - kbSize.height - textHeight;
    if (sub.y > offset_y) {
        int move_h = sub.height + textView.frame.size.height + (sub.y - offset_y - sub.height);

        screenView.frame = CGRectMake(screenRect.origin.x, screenRect.origin.y - move_h, screenRect.size.width, screenRect.size.height);
        return;
    }
    
}

-(void)keyboardWillHide: (NSNotification*) notification
{
    
    oldKeyboardHeight = 0;
    
    UIView* textView = getCurrentView();
    if (!textView)
        return;
    
    NSDictionary* keyboardInfo = [notification userInfo];
    NSValue* keyboardFrame = [keyboardInfo objectForKey:UIKeyboardFrameEndUserInfoKey];
    CGSize kbSize = [keyboardFrame CGRectValue].size;
    
    int textHeight = getTextInputHeight();
    
    if ([textView isKindOfClass:[UITextView class]]) {
        float h = UIApplication.sharedApplication.keyWindow.frame.size.height;
        textView.frame = CGRectMake(0, 0, 0, 0);
        
        if (@available(iOS 11.0, *)) {
            UIEdgeInsets edges = [UIApplication sharedApplication].keyWindow.safeAreaInsets;
            textBgView.frame = CGRectMake(0, 0, kbSize.width, h - kbSize.height);
            textBgView.hidden = YES;
            
            textView.frame = CGRectMake(0, 0, 0, 0);
        }
        return;
    }
    
    int offset_x = 0;
    if (@available(iOS 11.0, *)) {
        UIEdgeInsets edges = [UIApplication sharedApplication].keyWindow.safeAreaInsets;
        if (edges.right != 0) {
            offset_x = edges.right - 15;
        }
    }
    
    UIView* screenView = (UIView*)cocos2d::Application::getInstance()->getView();
    CGRect screenRect = screenView.frame;
    
    int offset_y = screenRect.size.height - kbSize.height - textHeight;
    if (sub.y > offset_y) {
        int move_h = sub.height + textView.frame.size.height + (sub.y - offset_y - sub.height);
        
        screenView.frame = CGRectMake(0, 0, screenRect.size.width, screenRect.size.height);
        return;
    }
    
    screenView.frame = CGRectMake(0, 0, screenRect.size.width, screenRect.size.height);
}

-(void)keyboardWillChangeFrame:(NSNotification *)notification {
    UIView* screenView = (UIView*)cocos2d::Application::getInstance()->getView();
    screenView.frame = CGRectMake(0, 0, screenView.frame.size.width, screenView.frame.size.height);
    UIView* textView = getCurrentView();
    CGRect screenRect = screenView.frame;
    screenView.frame = CGRectMake(0, 0, screenRect.size.width, screenRect.size.height);
    int textHeight = getTextInputHeight();
    
    NSDictionary* keyboardInfo = [notification userInfo];
    NSValue* keyboardFrame = [keyboardInfo objectForKey:UIKeyboardFrameEndUserInfoKey];
    CGSize kbSize = [keyboardFrame CGRectValue].size;
    
    if ([textView isKindOfClass:[UITextView class]]) {
        float h = UIApplication.sharedApplication.keyWindow.frame.size.height;
        textView.frame = CGRectMake(0, 0, 0, 0);
        
        if (@available(iOS 11.0, *)) {
            UIEdgeInsets edges = [UIApplication sharedApplication].keyWindow.safeAreaInsets;
            textBgView.frame = CGRectMake(0, 0, kbSize.width, h - kbSize.height);
            textBgView.hidden = NO;
            
        }
        return;
    }
    
    if (oldKeyboardHeight != kbSize.height) {
        int move_h = oldKeyboardHeight - kbSize.height;
        screenView.frame = CGRectMake(screenRect.origin.x, screenRect.origin.y + move_h, screenRect.size.width, screenRect.size.height);
        oldKeyboardHeight = kbSize.height;
    }
}
@end

@implementation TextFieldDelegate
- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
    if (textField.keyboardType != UIKeyboardTypeDecimalPad) {
        return YES;
    }
    
    if ([string isEqualToString:@"."]) {
        if ([textField.text containsString:@"."]) {
            return NO;
        }
    }
    
    return YES;
}

- (void)textFieldDidChange:(UITextField *)textField
{
    if (textField.markedTextRange != nil)
        return;

    if (textField.text.length > g_maxLength)
        textField.text = [textField.text substringToIndex:g_maxLength];

    callJSFunc("input", [textField.text UTF8String]);
    setText(textField.text);
}

-(BOOL) textFieldShouldReturn:(UITextField *)textField
{
    cocos2d::EditBox::complete();
    return YES;
}

- (void)textFieldDidBeginEditing:(UITextField *)textField
{
    if (textField.keyboardType == UIKeyboardTypeASCIICapableNumberPad || textField.keyboardType == UIKeyboardTypeDecimalPad) {
        NSString *newText = [CommaTool changeNumberNormalFormatter:textField.text];
        setText(newText);
    }
}
@end

@implementation ButtonHandler
-(IBAction) buttonTapped:(UIButton *)button
{
    const std::string text([getCurrentText() UTF8String]);
    callJSFunc("confirm", text);
    if (!g_confirmHold)
        cocos2d::EditBox::complete();
}
@end


@implementation TextViewDelegate
- (BOOL) textView:(UITextView *)textView shouldChangeTextInRange:(NSRange)range replacementText:(NSString *)text
{
    if([text isEqualToString:@"\n"]){
        [textView resignFirstResponder];
        return NO;
    }
    return YES;
}

- (void)textViewDidChange:(UITextView *)textView
{
    if (textView.markedTextRange != nil)
        return;

    if (textView.text.length > g_maxLength)
        textView.text = [textView.text substringToIndex:g_maxLength];

    callJSFunc("input", [textView.text UTF8String]);
    setText(textView.text);
}
@end

@implementation CommaTool
+(NSString*)changeNumberCommaFormatter:(NSString*)str{
    NSString *numString = [NSString stringWithFormat:@"%@",str];
    NSNumberFormatter *formatter = [[NSNumberFormatter alloc]init];
    NSNumber *number = [formatter numberFromString:numString];
    formatter.numberStyle = NSNumberFormatterDecimalStyle;
    NSString *string = [formatter stringFromNumber:number];
    NSLog(@"numberFormatter == %@",string);
    if(IsStrEmpty(string)) {
        return str;
    }
    return string;
}

+(NSString*)changeNumberNormalFormatter:(NSString*)str{
    str = [str stringByReplacingOccurrencesOfString:@"," withString:@""];
    if ([str length] <= 0) {
        return str;
    }
    NSString *theLast = [str substringFromIndex:[str length]-1];
    if ([theLast isEqualToString:@"K"]) {
        str = [str stringByReplacingCharactersInRange:NSMakeRange([str length]-1, 1) withString:@"000"];
    }
    
    NSString *numString = [NSString stringWithFormat:@"%@",str];
    NSNumberFormatter *formatter = [[NSNumberFormatter alloc]init];
    NSNumber *number = [formatter numberFromString:numString];
    formatter.numberStyle = NSNumberFormatterNoStyle;
    NSString *string = [formatter stringFromNumber:number];
    NSLog(@"numberFormatter == %@",string);
    if(IsStrEmpty(string)) {
        return str;
    }
    return string;
}
@end

/*************************************************************************
 Implementation of EditBox.
 ************************************************************************/


NS_CC_BEGIN

void EditBox::show(const cocos2d::EditBox::ShowInfo& showInfo)
{
    g_maxLength = showInfo.maxLength;
    g_isMultiline = showInfo.isMultiline;
    g_confirmHold = showInfo.confirmHold;
    
    [(CCEAGLView*)cocos2d::Application::getInstance()->getView() setPreventTouchEvent:true];
    addKeyboardEventLisnters();
    addTextInput(showInfo);
}

void EditBox::hide()
{
    UIView* view = getCurrentView();
    if (view)
    {
        [view removeFromSuperview];
        [view resignFirstResponder];
    }
    
    [(CCEAGLView*)cocos2d::Application::getInstance()->getView() setPreventTouchEvent:false];
    removeKeyboardEventLisnters();
}

void EditBox::complete()
{
    NSString* text = getCurrentText();
    callJSFunc("complete", [text UTF8String]);
    EditBox::hide();
}

NS_CC_END
