#import <Cocoa/Cocoa.h>

extern "C" void MacOS_OpenURL(const char* url)
{
    if (!url) return;
    
    NSString *urlString = [NSString stringWithUTF8String:url];
    NSURL *nsurl = [NSURL URLWithString:urlString];
    if (nsurl) {
        [[NSWorkspace sharedWorkspace] openURL:nsurl];
    }
}
