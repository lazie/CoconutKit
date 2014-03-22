//
//  UIView+HLSViewBinding.h
//  CoconutKit
//
//  Created by Samuel Défago on 18.06.13.
//  Copyright (c) 2013 Hortis. All rights reserved.
//

/**
 * Usually, when you have to display some value on screen, and if you are using Interface Builder to design 
 * your user interface, you have to create and bind an outlet. Though this process is completely straightforward,
 * this tends to clutter you code and becomes increasingly boring, especially when the number of values to 
 * display is large.
 *
 * CoconutKit view bindings allow you to bind values to views directly in Interface Builder, via user-defined
 * runtime attributes instead of outlets. Two attributes are available to this purpose:
 *   - bindKeyPath: The keypath pointing at the value to which the view will be bound. This can be any kind of 
 *                  keypath, even one containing keypath operators
 *   - bindFormatter: Values to be displayed by bound views must have an appropriate type, most of the time
 *                    NSString. The classes supported for binding to a view are returned by the bound view
 *                    +supportedBindingClasses class method (if not implemented, defaults to NSString). If 
 *                    bindKeyPath returns a non-supported kind of object (say of class SomeClass), you must 
 *                    provide the name of an instance formatter method 'methodName:', which can either be an 
 *                    instance method with prototype
 *                      - (NSFormatter *)methodName
 *                    or a class method with prototype
 *                      + (NSFormatter *)classMethodName
 *                    returning an NSFormatter transforming the object into another one with supported type. 
 *                    These methods are looked up along the responder chain, as described below. Alternatively, 
 *                    you can provide a global class formatter method '[SomeClass methodName]', returning an
 *                    NSFormatter object. 
 *                    Formatters are required when the type of the value returned by the key path does not
 *                    match one of the supported types, but can also be used to further format any value. For
 *                    example, if a view supports binding to NSNumber, and if the key path returns an NSNumber,
 *                    you might still want to use a formatter to round the value, multiply it with some constant,
 *                    etc.
 *                    If you need to implement a custom formatter, and if you only need bindings for displaying
 *                    formatted values (not to parsing input), you can only implement the NSFormatter dedicated
 *                    method (i.e. formatting, not parsing)
 *
 * With no additional measure, keypath lookup is performed along the responder chain, starting with the view
 * bindKeyPath has been set on, and stopping at the first encountered view controller (if any is found). View
 * controllers namely define a local context, and it does not make sense to proceed further along the responder
 * chain. The same is true for formatter selector lookup (at each step along the responder chain, instance
 * method existence is tested first, then class method existence).
 *
 * Often, though, values to be bound stem from a model object, not from the responder chain. In such cases,
 * you must call -bindToObject: on the view to be bound, passing it the object to be bound against. The keypath 
 * you set must be be valid for this object. Formatter lookup is first made on the object class itself (instance, 
 * then class method), then along the responder chain (instance, then class method, again stopping at view controller 
 * boundaries), except if a global class formatter is used
 *
 * To summarize, formatter lookup for a method named 'methodName:' is performed from the most specific to 
 * the most generic context, within the boundaries of a view controller (if any), as follows:
 *   - instance method -methodName: on bound object (if -bindToObject: has been used)
 *   - class method +methodName: on bound object (if -bindToObject: has been used)
 *   - for each responder along the responder chain starting with the bound view:
 *       - instance method -methodName: on the responder object
 *       - class method +methodName: on the responder object
 * In addition, global formatter names can be provided in the form of class methods '+[SomeClass methodName:]'
 *
 * The binding information is resolved as late as possible (usually when the view is displayed), when the whole
 * repsonder chain context is available. This information is then stored for efficient later use. The view is
 * not updated automatically when the underlying bound objects changes, this has to be done manually:
 *   - when the object is changed, call -bindToObject: to set bindings with the new object
 *   - if the object does not change but has different values for its bounds properties, simply call -refreshBindings
 *     to reflect the new values which are available
 *
 * It would be painful to call -bindToObject:, -refreshBindings:, etc. on all views belonging to a view hierarchy
 * when bindings must be established or refreshed. For this reason, those calls are made recursively. This
 * means you can simply call one of those methods at the top of the view hierarchy (or even on the view controller 
 * itself, see UIViewController+HLSViewBinding.h) to bind or refresh the whole associated view hierarchy. Note that 
 * each view class decides whether it recursively binds or refreshes its subviews (this behavior is controlled via
 * the HLSViewBinding protocol)
 *
 * In most cases, you want to bind a single view hierarchy to a single object. But you can also have separate 
 * view hierarchies within the same view controller context if you want, each one bound to a different object.
 * Nesting is possible as well, but can be more subtle and depends on the order in which -bindToObject: is 
 * called. Though you should in general avoid such designs, you can still bind nested views correctly by 
 * calling -bindToObject: on parent views first.
 *
 * TODO: Document validation and sync in the other direction (when available)
 *       Warning:
 *         - bidirectional bindings are not compatible with all keypaths (e.g. keypaths containing operators). This
 *           has to be checked when trying to resolve bindings
 *
 * Here is how UIKit view classes play with bindings:
 *   - UILabel: The label displays the value which the keypath points at. Bindings are not recursive. The only 
 *              supported class is NSString
 *   - UIProgressView: The progress view displays the value which the keypath points at, and dragging the slider
 *                     changes the underlying value. Bindings are not recursive. The only supported class is NSNumber 
 *                     (treated as a float)
 *   - UITableView: No direct binding is available, and bindings are not recursive. You can still bind table view 
 *                  cells and headers created from nibs, though
 *   - UISwitch: The switch displays the value which the keypath points at, and toggling the switch changes the
 *               underlying value. Bindings are not recursive. The only supported class is NSNumber (treated as a 
 *               boolean)
 *   - UITextField: <explain>
 *   - UITextView: <explain>
 *   - UIWebView: <explain>
 *
 * You can customize the binding behavior for other UIView subclasses (whether these classes are your own or stem
 * from a 3rd party library) by implementing the HLSViewBinding protocol. For 3rd party classes, this is best
 * achieved using a category conforming to HLSViewBinding (see CoconutKit UILabel+HLSViewBinding for an example)
 */

/**
 * This protocol can be implemented by UIView subclasses to customize binding behavior 
 */
@protocol HLSViewBinding <NSObject>

@optional

/**
 * Return the list of classes supported for bindings. If this method is not implemented, the supported types default
 * to NSString only
 */
+ (NSArray *)supportedBindingClasses;

/**
 * UIView subclasses which want to provide bindings MUST implement this method. Its implementation should update the 
 * view according to the value which is received as parameter (if this value can be something else than an NSString,
 * be sure to implement the +supportedBindingClasses method as well). If a UIView class does not implement this method,
 * bindings will not be available for it.
 *
 * You can call -bindToObject:, -refreshBindings:, etc. on any view, whether it actually implement -updateViewWithValue:
 * or not. This will recursively traverse its view hierarchy wherever possible (see -bindsSubviewsRecursively) and
 * perform binding resolution for views deeper in its hierarchy
 */
- (void)updateViewWithValue:(id)value;

/**
 * UIView subclasses can implement this method to return YES if subviews must be updated recursively when the
 * receiver is updated. When not implemented, the default behavior is YES
 */
- (BOOL)bindsSubviewsRecursively;

#if 0
/**
 * UIView subclasses which accept user input and want bindings to automatically update the underlying model (as
 * given by the associated keyPath) can implement the following method. This method must be implemented along
 * -updateViewWithValue:, which is required for bindings to be enabled
 */
- (void)updateModelWithText:(NSString *)text;

// TODO: Optional validation (see Key-Value coding programming guide, -validate<field>:error:). Probably introduce
//       a boolean user-defined runtime attribute setting whether or not validation must occur (by default should
//       be YES, i.e. if a validation method exists applies it)
#endif

#if 0
Formatting: several approaches, must allow all of them with the same resolution mechanism:

- for model -> view: method <objectA>From<Object>:
- for view -> model: method <objectB>From<ObjectA>:
- method returning an NSFormatter instance

#endif

@end

/**
 * View binding additions. All methods can be called whether a view implements binding support or not. When calling
 * one of those methods on a view, the view hierarchy rooted at it is traversed, until views which do not support
 * recursion are found (see HLSViewBinding protocol), or until a view controller boundary is reached
 */
@interface UIView (HLSViewBinding)

/**
 * Bind the view (and recursively the view hierarchy rooted at it) to a given object (can be nil). During view 
 * hierarchy traversal, keypaths and formatters set via user-defined runtime attributes will be used to automatically 
 * fill those views which implement binding support
 */
- (void)bindToObject:(id)object;

/**
 * Refresh the value displayed by the view, recursively traversing the view hierarchy rooted at it. If forced is set
 * to NO, bindings are not checked again (i.e. formatters are not resolved again), values are only updated using
 * information which has been cached the first time bindings were successfully checked. If you want to force bindings
 * to be checked again first (i.e. formatters to be resolved again), set forced to YES
 */
- (void)refreshBindingsForced:(BOOL)forced;

@end
