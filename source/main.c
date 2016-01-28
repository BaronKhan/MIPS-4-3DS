#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <3ds.h>
#include <mips.h>
#include <dirent.h>

#include <stdint.h>

uint32_t f_fibonacci(uint32_t n)
{
	if(n<=1)
		return n;
	return f_fibonacci(n-1)+f_fibonacci(n-2);
}


int main()
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);
                char* srcName;
                //open directory
                DIR *pdir;
		struct dirent *pent;
		int err = 0;

		pdir=opendir("/3ds/mips43ds/");
                printf("Files in /3ds/mips43ds/ : \n");
		if (pdir){
                        char dir_name[50][512];
                        int i = 0;
			while ((pent=readdir(pdir))!=NULL) {
                                if(strcmp(".", pent->d_name) == 0 || strcmp("..", pent->d_name) == 0)
                                        continue;
                                if(pent->d_type != DT_DIR) {
                                        printf("%d. %s\n", i+1, pent->d_name);
                                        strcpy(dir_name[i], pent->d_name);
                                        i++;
                                }

			}


			//select file
			printf("Please select a file to load (Left/Right keys):\n");
			int select = 0; int selected = 0;
			printf("< %d. %s >\n", 1, dir_name[0]);
			while(selected == 0) {
                                gspWaitForVBlank();
                                hidScanInput();
                                u32 kDown = hidKeysDown();
                                if (kDown & KEY_DRIGHT) {
                                        select++;
                                        if (select < 0)
                                                select = i;
                                        if (select > i)
                                                select = 0;
                                        printf("< %d. %s >\n", select+1, dir_name[select]);

                                }
                                else if (kDown & KEY_DLEFT) {
                                        select--;
                                        if (select < 0)
                                                select = i;
                                        if (select > i)
                                                select = 0;
                                        printf("< %d. %s >\n",select+1, dir_name[select]);
                                }
                                if (kDown & KEY_A) {
                                        selected = true;
                                        srcName = dir_name[select];
                                }
                                if (kDown & KEY_START){
                                        romfsExit();
                                        gfxExit();
                                        return 0;
                                }
			}
			printf("\nSelected file %s.\n", dir_name[select]);

			closedir(pdir);


		} else {
			iprintf ("opendir() failure; terminating\nPress Start to Exit\n");
			while (1) {
			        u32 kDown = hidKeysDown();
                                gspWaitForVBlank();
                                hidScanInput();
                                if (kDown & KEY_START){
                                        romfsExit();
                                        gfxExit();
                                        return 0;
                                }
			}
		}



                mips_mem_h m=mips_mem_create_ram(0x20000, 4);
                mips_cpu_h c=mips_cpu_create(m);

                //char* srcName = "test_mips_fibonacci.bin";
                FILE *src=fopen(srcName,"rb");
                    if(!src){
                        fprintf(stderr, "Cannot load source file '%s', try specifying the relative path to '%s'.\nPress Start to Exit\n", srcName, srcName);
                        while (1) {
			        u32 kDown = hidKeysDown();
                                gspWaitForVBlank();
                                hidScanInput();
                                if (kDown & KEY_START){
                                        romfsExit();
                                        gfxExit();
                                        return 0;
                                }
			}
                    }

                    uint32_t v;
                    uint32_t offset=0;
                    while(1==fread(&v, 4, 1, src)){
                        if(mips_mem_write(m, offset, 4, (uint8_t*)&v)){
                            fprintf(stderr, "Memory error while loading binary.\nPress Start to Exit\n");
                            while (1) {
			        u32 kDown = hidKeysDown();
                                gspWaitForVBlank();
                                hidScanInput();
                                if (kDown & KEY_START){
                                        romfsExit();
                                        gfxExit();
                                        return 0;
                                }
                            }
                        }
                        offset+=4;
                    }
                    fprintf(stderr, "Loaded %d bytes of binary at address 0.\n", offset);

                    fclose(src);

                        uint32_t n=12;  // Value we will calculate fibonacci of

                        uint32_t sentinelPC=0x10000000;

                        mips_cpu_set_register(c, 31, sentinelPC);   // set return address to something invalid
                        mips_cpu_set_register(c, 4, n);             // Set input argument
                        mips_cpu_set_register(c, 29, 0x1000);       // Create a stack pointer

                        uint32_t steps=0;
                        fprintf(stderr, "Executing... ");
                        while(!mips_cpu_step(c)){
                        //fprintf(stderr, "Step %d.\n", steps);

                        ++steps;
                        uint32_t pc;
                        mips_cpu_get_pc(c, &pc);
                        if(pc==sentinelPC)
                            break;
                        }
                        fprintf(stderr, "Done!\n");

                        //uint32_t fib_n;
                        //mips_cpu_get_register(c, 2, &fib_n);    // Get the result back

                        //uint32_t fib_n_ref=f_fibonacci(n);

                        //fprintf(stderr, "fib(%d) = %d, expected = %d\n", n, fib_n, fib_n_ref);

                        fprintf(stderr, "Final register values:\n");
                        uint32_t reg_val;
                        for (int i=0; i < 32; i+=2) {
                                mips_cpu_get_register(c, i, &reg_val);    // Get the result back
                                fprintf(stderr, "$%d = %d,\t\t\t\t\t\t", i, reg_val);
                                mips_cpu_get_register(c, i+1, &reg_val);    // Get the result back
                                fprintf(stderr, "$%d = %d\n", i+1, reg_val);

                        }

                        fprintf(stderr, "\n\nPress Start to Exit\n");



	//}

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu
	}

	romfsExit();
	gfxExit();
	return 0;
}
