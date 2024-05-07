#include "terminalprofile.h"

#import <Foundation/Foundation.h>
#import <OpenDirectory/OpenDirectory.h>

QString TerminalProfile::defaultShell() {
    auto odNode = [ODNode nodeWithSession:[ODSession defaultSession] name:@"/Local/Default" error:nil];
    auto odRecord = [odNode recordWithRecordType:kODRecordTypeUsers name:NSUserName() attributes:nil error:nil];
    auto values = [odRecord valuesForAttribute:kODAttributeTypeUserShell error:nil];
    return QString::fromNSString([values firstObject]);
}
