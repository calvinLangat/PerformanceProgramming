#include <stdio.h>
#include <inttypes.h>
#include <windows.h>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <cpuid.h>
#endif




typedef uint32_t u32;
typedef int32_t  s32;
typedef uint64_t u64;
typedef int64_t  s64;
typedef uint32_t b32;
typedef float    f32;
typedef double   f64;

u32 get_base_frequency() {
    u32 eax, ebx, ecx, edx;

#ifdef _MSC_VER
    int cpuInfo[4];
    __cpuid(cpuInfo, 0x16); // Call CPUID with function 0x16
    eax = cpuInfo[0];
#else
    __cpuid(0x16, eax, ebx, ecx, edx); // Call CPUID with function 0x16
#endif

    return eax; // Base frequency in MHz
}

u64 get_cpu_frequency() {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                     "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 
                     0, 
                     KEY_READ, 
                     &hKey) == ERROR_SUCCESS) {
        DWORD data, dataSize = sizeof(data);
        if (RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE)&data, &dataSize) == ERROR_SUCCESS) {
            printf("Processor Base Frequency: %u MHz\n", data);
            return data;
        } else {
            printf("Unable to read frequency.\n");
            return 0xFFFFFFFF;
        }
        RegCloseKey(hKey);
    } else {
        printf("Unable to open registry key.\n");
        return 0XFFFFFFFF;
    }
}

const f64 timestep = 0.0000001;
const f32 g = 9.81;

u64 timestep_counter;
u64 last_time_ticks;

struct particle{
	f32 mass;
	f32 posX;
	f32 posY;
	f32 velocity;
	f32 direction;
};

void accelerate(struct particle *particle, f32 *force)
{
	f32 accel = *force / particle->mass;
}

int main(int argc, char const *argv[])
{
	// u32 base_freq = get_base_frequency();
    // if (base_freq == 0) {
    //     printf("Base frequency is not available on this processor.\n");
    // } else {
    //     printf("Processor Base Frequency: %u MHz\n", base_freq);
    // }

	u64 clock_freq = get_cpu_frequency() * 1000000;
	struct particle particle1;
	particle1.posY = 10.0;
	u64 tick_diff;
	printf("Starting...\n");
	last_time_ticks = __builtin_ia32_rdtsc();
	char buff[4092];
	u32 offset = 0; // Start at the beginning of the buffer

	while(1)
	{
		tick_diff = __builtin_ia32_rdtsc() - last_time_ticks;
		f64 step_calculated = tick_diff / (f64)clock_freq;
		if (step_calculated >= timestep)
		{
			last_time_ticks = __builtin_ia32_rdtsc();
			particle1.posY -= 0.1;
			//printf("%f, %f\n", particle1.posY, tick_diff / (f32)clock_freq);
			int written = snprintf(buff + offset, sizeof(buff) - offset, "%f, %llu\n", 
                       particle1.posY, tick_diff);
			if (written < 0 || written >= sizeof(buff) - offset) {
			    fprintf(stderr, "Buffer overflow or error occurred!\n");
			}
			offset += written; // Update the offset
		}
		else
		{
			continue;
		}

		if(particle1.posY <=0) break;
	}

	printf("%s", buff);
	printf("Stopped.\n");
	return 0;
}