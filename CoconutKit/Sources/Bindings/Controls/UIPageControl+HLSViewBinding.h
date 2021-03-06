//
//  Copyright (c) Samuel Défago. All rights reserved.
//
//  Licence information is available from the LICENCE file.
//

#import "UIView+HLSViewBindingImplementation.h"

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

/**
 * Binding support for UIPageControl:
 *   - binds to NSNumber (integer) or NSInteger model values
 *   - displays and updates the underlying model value
 *   - does not animate updates
 * - check (if not disabled via bindInputChecked) and update the value each time it is changed
 */
@interface UIPageControl (HLSViewBinding) <HLSViewBindingImplementation>

@end
