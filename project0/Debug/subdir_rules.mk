################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
emg_mock.o: C:/Users/Symae/Downloads/ECE480/MCU_OLED_UI_LOGIC/src/emg_mock.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs2031/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mlittle-endian -mthumb -I"C:/ti/TivaWare_C_Series-2.2.0.295/inc" -I"C:/ti/TivaWare_C_Series-2.2.0.295" -I"C:/Users/Symae/Downloads/ECE480/MCU_OLED_UI_LOGIC/include" -DPART_TM4C123GH6PM -Wall -fdata-sections -ffunction-sections -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

game.o: C:/Users/Symae/Downloads/ECE480/MCU_OLED_UI_LOGIC/src/game.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs2031/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mlittle-endian -mthumb -I"C:/ti/TivaWare_C_Series-2.2.0.295/inc" -I"C:/ti/TivaWare_C_Series-2.2.0.295" -I"C:/Users/Symae/Downloads/ECE480/MCU_OLED_UI_LOGIC/include" -DPART_TM4C123GH6PM -Wall -fdata-sections -ffunction-sections -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

gfx.o: C:/Users/Symae/Downloads/ECE480/MCU_OLED_UI_LOGIC/src/gfx.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs2031/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mlittle-endian -mthumb -I"C:/ti/TivaWare_C_Series-2.2.0.295/inc" -I"C:/ti/TivaWare_C_Series-2.2.0.295" -I"C:/Users/Symae/Downloads/ECE480/MCU_OLED_UI_LOGIC/include" -DPART_TM4C123GH6PM -Wall -fdata-sections -ffunction-sections -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

main.o: C:/Users/Symae/Downloads/ECE480/MCU_OLED_UI_LOGIC/src/main.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs2031/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mlittle-endian -mthumb -I"C:/ti/TivaWare_C_Series-2.2.0.295/inc" -I"C:/ti/TivaWare_C_Series-2.2.0.295" -I"C:/Users/Symae/Downloads/ECE480/MCU_OLED_UI_LOGIC/include" -DPART_TM4C123GH6PM -Wall -fdata-sections -ffunction-sections -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

process.o: C:/Users/Symae/Downloads/ECE480/MCU_OLED_UI_LOGIC/src/process.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs2031/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mlittle-endian -mthumb -I"C:/ti/TivaWare_C_Series-2.2.0.295/inc" -I"C:/ti/TivaWare_C_Series-2.2.0.295" -I"C:/Users/Symae/Downloads/ECE480/MCU_OLED_UI_LOGIC/include" -DPART_TM4C123GH6PM -Wall -fdata-sections -ffunction-sections -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

ssd1351.o: C:/Users/Symae/Downloads/ECE480/MCU_OLED_UI_LOGIC/src/ssd1351.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs2031/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mlittle-endian -mthumb -I"C:/ti/TivaWare_C_Series-2.2.0.295/inc" -I"C:/ti/TivaWare_C_Series-2.2.0.295" -I"C:/Users/Symae/Downloads/ECE480/MCU_OLED_UI_LOGIC/include" -DPART_TM4C123GH6PM -Wall -fdata-sections -ffunction-sections -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs2031/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mlittle-endian -mthumb -I"C:/ti/TivaWare_C_Series-2.2.0.295/inc" -I"C:/ti/TivaWare_C_Series-2.2.0.295" -I"C:/Users/Symae/Downloads/ECE480/MCU_OLED_UI_LOGIC/include" -DPART_TM4C123GH6PM -Wall -fdata-sections -ffunction-sections -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

timer.o: C:/Users/Symae/Downloads/ECE480/MCU_OLED_UI_LOGIC/src/timer.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs2031/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mlittle-endian -mthumb -I"C:/ti/TivaWare_C_Series-2.2.0.295/inc" -I"C:/ti/TivaWare_C_Series-2.2.0.295" -I"C:/Users/Symae/Downloads/ECE480/MCU_OLED_UI_LOGIC/include" -DPART_TM4C123GH6PM -Wall -fdata-sections -ffunction-sections -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


