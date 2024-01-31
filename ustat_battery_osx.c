
// cc b.c -framework IOKit -framework CoreFoundation
#include <IOKit/IOKitLib.h>
#include <IOKit/IOTypes.h>
#include <IOKit/ps/IOPSKeys.h>
#include <IOKit/ps/IOPowerSources.h>

static int _get_battery_info(struct ustat_battery* bi) {

    CFTypeRef psi = IOPSCopyPowerSourcesInfo();
    CFArrayRef psil = IOPSCopyPowerSourcesList(psi);
    CFTypeRef ps = CFArrayGetValueAtIndex(psil, 0);
    CFDictionaryRef psid = IOPSGetPowerSourceDescription(psi, ps);

    CFBooleanRef isCharging =
        CFDictionaryGetValue(psid, CFSTR(kIOPSIsChargingKey));
    if (isCharging != NULL && CFBooleanGetValue(isCharging)) {
        bi->is_charging = 1;
    }

    CFNumberRef val =
        CFDictionaryGetValue(psid, CFSTR(kIOPSCurrentCapacityKey));
    if (val != NULL) {
        int bl = 0;
        CFNumberGetValue(val, kCFNumberIntType, &bl);
        bi->level = bl;
    }

    return 0;
}
