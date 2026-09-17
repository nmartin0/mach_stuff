#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <mach/mach_types.h>
// #define SInt32 UNIXSInt32
// #include <IOKit/IOTypes.h>
// #undef SInt32
#include <IOKit/IOKitLib.h>
#include <IOKit/IOReturn.h>
#include <mach/mach_init.h>

#include <CoreServices/CoreServices.h>

#if 0
typedef void *CFNumberRef;
typedef void *CFDictionaryRef;
typedef void *CFMutableDictionaryRef;
extern mach_port_t bootstrap_port;
#define kNilOptions 0
#define kCFAllocatorDefault 0
#define kCFNumberSInt32Type 32
#endif

void printusage(char *name);
int setdebug(int debugval, char *devname);

int main(int argc, char *argv[])
{
    int debugval;

    if (argc != 3) {
	printusage(argv[0]);
	exit(-1);
    }
    debugval = atoi(argv[2]);

    return (setdebug(debugval, argv[1]));
}


void printusage(char *name)
{

    if (name) {
	printf("Usage: %s device value\n\n", name);
    } else {
	printf("Usage: swim3_setdebug device value\n\n");
    }

    printf("Where value is the OR of the following values:\n\n");

    printf("DEBUG_GENERAL = 0x1\n");
    printf("DEBUG_VERBOSE = 0x2\n");
    printf("DEBUG_WAIT    = 0x4\n");
    printf("DEBUG_REGS    = 0x8\n");

    printf("\nYou probably want 0x3 for most debugging.\n");

    return;
}


int do_set(int debugval, char *devname);
int setdebug(int debugval, char *devname)
{
    int done_one = 0;

    printf("Setting debug value for device \"%s\" to:\n", devname);

    if (debugval & 0x1) {
	printf("DEBUG_GENERAL ");
	done_one = 1;
    }
    if (debugval & 0x2) {
	if (done_one) printf("| ");
	printf("DEBUG_VERBOSE ");
	done_one = 1;
    }
    if (debugval & 0x4) {
	if (done_one) printf("| ");
	printf("DEBUG_WAIT ");
	done_one = 1;
    }
    if (debugval & 0x8) {
	if (done_one) printf("| ");
	printf("DEBUG_REGS ");
	done_one = 1;
    }

    printf("\n");

    return do_set(debugval, devname);
}


void do_error(char *functionname)
{
fprintf(stderr, "Call to %s failed.\n", functionname);
}


#if 1
int do_set(int debugval, char *debugname)
{
    mach_port_t master_port;
#if 0
    io_registry_entry_t ioregent;
    CFDictionaryRef dict;
#endif
    kern_return_t retval = -2;
    io_iterator_t it;
    io_object_t obj;
    CFTypeRef prop;
    // CFStringRef stringref;
    CFNumberRef val;
    int debugvalstore = debugval;

    if (IOMasterPort(bootstrap_port, &master_port) != KERN_SUCCESS)
	do_error("IOMasterPort");
    if (IORegistryCreateIterator(master_port, kIOServicePlane, 
	kIORegistryIterateRecursively, &it) != KERN_SUCCESS)
	    do_error("IORegistryCreateIterator");
    while (obj = IOIteratorNext(it)) {
	prop = IORegistryEntryCreateCFProperty(obj, 
	    CFSTR("IOClass"), kCFAllocatorDefault, kNilOptions);
	if (prop) {
	    const char *cstr;

	    if (CFGetTypeID(prop) != CFStringGetTypeID()) continue;
	    cstr = CFStringGetCStringPtr(prop, kCFStringEncodingMacRoman);
	    if (!strcmp(cstr, "org_mklinux_iokit_swim3_driver")) {
#if 0
		if (IORegistryEntryCreateCFProperties(obj,
		    (CFMutableDictionaryRef *) &dict, kCFAllocatorDefault,
		    kNilOptions) == KERN_SUCCESS) {
			val = CFNumberCreate(kCFAllocatorDefault,
			    kCFNumberSInt32Type, (SInt32 *)&debugvalstore);
			CFDictionarySetValue(dict, CFSTR("SWIM3 Debug"),
			    val);
		    if ((retval=IORegistryEntrySetCFProperties(obj, dict)) !=
			KERN_SUCCESS) {
			    do_error("IORegistryEntrySetCFProperties");
			    fprintf(stderr, "Retval = %d (0x%x)\n",
				retval, retval);
		    }
		} else do_error("IORegistryEntryCreateCFProperties");
#else
		val = CFNumberCreate(kCFAllocatorDefault,
		    kCFNumberSInt32Type, (SInt32 *)&debugvalstore);
		if ((retval=IORegistryEntrySetCFProperty(obj,
		    CFSTR("SWIM3 Debug"), val)) != KERN_SUCCESS) {
			    do_error("IORegistryEntrySetCFProperties");
			    fprintf(stderr, "Retval = %d (0x%x)\n",
				retval, retval);
			    perror("Hi");
		}
#endif
	    }
printf("k"); fflush(stdout);
	}
printf("l"); fflush(stdout);
    }
printf("m"); fflush(stdout);
    return retval;
}
#else
int do_set(int debugval, char *devname)
{
    mach_port_t master_port;
    CFMutableDictionaryRef dict;
    io_iterator_t it;
    io_object_t obj;
    CFNumberRef val;
    int debugvalstore = debugval;

    if (IOMasterPort(bootstrap_port, &master_port) != KERN_SUCCESS)
	do_error("IOMasterPort");

    dict = IOBSDNameMatching(master_port, kNilOptions, devname);
    if (IOServiceGetMatchingServices(master_port, dict, &it) != KERN_SUCCESS) {
	do_error("IOServiceGetMatchingServices");
	return;
    }
    obj = IOIteratorNext(it);
    if (!obj) {
	do_error("IOIteratorNext");
	return;
    }
		if (IORegistryEntryCreateCFProperties(obj,
		    (CFMutableDictionaryRef *) &dict, kCFAllocatorDefault,
		    kNilOptions) == KERN_SUCCESS) {
			val = CFNumberCreate(kCFAllocatorDefault,
			    kCFNumberSInt32Type, (SInt32 *)&debugvalstore);
			return CFDictionarySetValue(dict, CFSTR("SWIM3 Debug"),
			    val);
		    if (IORegistryEntrySetCFProperties(obj, dict) !=
			KERN_SUCCESS) {
			    do_error("IORegistryEntrySetCFProperties");
		    }
		} else do_error("IORegistryEntryCreateCFProperties");
}
#endif
