//
//  S_CCActions.cpp
//  TestCocos2dX
//
//  Created by Rolando Abarca on 12/19/11.
//  Copyright (c) 2011 Zynga Inc. All rights reserved.
//

#include "S_CCActions.h"

JSClassRef js_S_CCAction_class;

using namespace cocos2d;

SCRIPTABLE_BOILERPLATE_IMP(S_CCAction)

JSStaticFunction* S_CCAction::jsStaticFunctions()
{
	static JSStaticFunction funcs[] = {
		{"isDone", S_CCAction::jsIsDone, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete},
		{"startWithTarget", S_CCAction::jsStartWithTarget, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete},
		{"stop", S_CCAction::jsStop, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete},
		{0, 0, 0}
	};
	
	return funcs;
}

bool S_CCAction::initWithContext(JSContextRef ctx, JSObjectRef obj, size_t argumentCount, const JSValueRef arguments[])
{
	jsObject = obj;
	return true;
}

JS_STATIC_FUNC_IMP(S_CCAction, jsIsDone)
{
	CCAction *act = (CCAction *)JSObjectGetPrivate(thisObject);
	if (act) {
		return JSValueMakeBoolean(ctx, act->isDone());
	}
	return JSValueMakeUndefined(ctx);
}

JS_STATIC_FUNC_IMP(S_CCAction, jsStartWithTarget)
{
	CCAction *act = (CCAction *)JSObjectGetPrivate(thisObject);
	if (act && argumentCount == 1) {
		JSObjectRef jsTarget = JSValueToObject(ctx, arguments[0], NULL);
		CCNode *target = (CCNode *)JSObjectGetPrivate(jsTarget);
		if (target) {
			act->startWithTarget(target);
		}
	}
	return thisObject;
}

JS_STATIC_FUNC_IMP(S_CCAction, jsStop)
{
	CCAction *act = (CCAction *)JSObjectGetPrivate(thisObject);
	if (act) {
		act->stop();
	}
	return thisObject;
}

# pragma mark - CCMoveBy

JSClassRef js_S_CCMoveBy_class;

SCRIPTABLE_BOILERPLATE_IMP(S_CCMoveBy)

JSStaticFunction* S_CCMoveBy::jsStaticFunctions()
{
	return NULL;
}

bool S_CCMoveBy::initWithContext(JSContextRef ctx, JSObjectRef obj, size_t argumentCount, const JSValueRef arguments[])
{
	// duration, point
	if (argumentCount == 2) {
		CCPoint *pt = (CCPoint *)JSObjectGetPrivate((JSObjectRef)arguments[1]);
		if (!CCMoveBy::initWithDuration(JSValueToNumber(ctx, arguments[0], NULL), *pt)) {
			return false;
		}
	}
	jsObject = obj;
	return true;
}

#pragma mark - CCSequence

JSClassRef js_S_CCSequence_class;

JSObjectRef S_CCSequence::jsConstructor(JSContextRef ctx, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	return NULL;
}

bool S_CCSequence::jsHasProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName)
{
	return false;
}

JSValueRef S_CCSequence::jsGetProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyNameJS, JSValueRef* exception)
{
	return JSValueMakeUndefined(ctx);
}


JSStaticFunction* S_CCSequence::jsStaticFunctions()
{
	static JSStaticFunction funcs[] = {
		{"actions", S_CCSequence::jsActions, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete},
		{0, 0, 0}
	};
	return funcs;
}

bool S_CCSequence::initWithContext(JSContextRef ctx, JSObjectRef obj, size_t argumentCount, const JSValueRef arguments[])
{
	return false;
}

/**
 * var seq = CCSequence.actions(action1, action2, action3, ...);
 * node.runAction(seq);
 */
JS_STATIC_FUNC_IMP(S_CCSequence, jsActions)
{
	if (argumentCount > 0) {
		int idx = 0;
		JSObjectRef prev = JSValueToObject(ctx, arguments[0], NULL);
		JSObjectRef pnow = NULL;
		while (idx < argumentCount - 1) {
			pnow = JSValueToObject(ctx, arguments[++idx], NULL);
			// need to create an action here, with prev & pnow
			CCFiniteTimeAction *cc1 = (CCFiniteTimeAction *)JSObjectGetPrivate(prev);
			CCFiniteTimeAction *cc2 = (CCFiniteTimeAction *)JSObjectGetPrivate(pnow);
			CCSequence *seq = CCSequence::actionOneTwo(cc1, cc2);

			JSObjectRef obj = JSObjectMake(ctx, js_S_CCSequence_class, NULL);
			JSObjectSetPrivate(obj, seq);
			prev = obj;
		}
		return prev;
	}
	return JSValueMakeUndefined(ctx);
}

#pragma mark - CCCallFunc

JSClassRef js_S_CCCallFunc_class;

SCRIPTABLE_BOILERPLATE_IMP(S_CCCallFunc)

JSStaticFunction* S_CCCallFunc::jsStaticFunctions()
{
	return NULL;
}

/**
 * target MUST be a CCNode
 * 
 * var callFunc = new CCCallFunc(function () {});  // <~ `this' in the function is the callFunc object
 */
bool S_CCCallFunc::initWithContext(JSContextRef ctx, JSObjectRef obj, size_t argumentCount, const JSValueRef arguments[])
{
	if (argumentCount == 1) {
		JSObjectRef func = JSValueToObject(ctx, arguments[0], NULL);
		if (!JSObjectIsFunction(ctx, func) || !CCCallFunc::initWithTarget(this)) {
			return false;
		}
		m_pCallFunc = callfunc_selector(S_CCCallFunc::callfunc);
		// store the callback on the object
		JSValueProtect(ctx, func);
		callback = func;
		
		jsObject = obj;
		return true;
	}
	return false;
}

S_CCCallFunc::~S_CCCallFunc()
{
	if (jsObject) {
		JSContextRef ctx = ScriptingCore::getInstance().getGlobalContext();
		JSValueUnprotect(ctx, callback);
	}
}

void S_CCCallFunc::callfunc()
{
	JSContextRef ctx = ScriptingCore::getInstance().getGlobalContext();
	JSObjectCallAsFunction(ctx, callback, NULL, 0, NULL, NULL);
}
