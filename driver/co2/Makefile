# 커널 모듈 이름
MODNAME := dms_co2

# 디바이스 트리 오버레이 정보
DTBO_SRC := scd4x.dts
DTBO_OUT := scd4x.dtbo
DTBODIR := /boot/overlays

# 커널 헤더 및 빌드 경로
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# 기본 빌드 대상
obj-m := $(MODNAME).o

# 커널 모듈 빌드
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# 디바이스 트리 오버레이 빌드
dtbo: $(DTBO_SRC)
	dtc -I dts -O dtb -@ -o $(DTBO_OUT) $(DTBO_SRC)

# 빌드 산출물 정리
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

# 자동 로드 설정 파일 생성
autoload:
	echo $(MODNAME) | sudo tee /etc/modules-load.d/$(MODNAME).conf > /dev/null

# 모듈/오버레이 설치 및 설정
install: all dtbo autoload
	@echo "Installing kernel module and device tree overlay..."
	sudo cp $(MODNAME).ko /lib/modules/$(shell uname -r)/kernel/drivers/i2c/
	sudo depmod -a
	sudo cp $(DTBO_OUT) $(DTBODIR)/
	sudo grep -q '^dtoverlay=scd4x' /boot/firmware/config.txt || echo 'dtoverlay=scd4x' | sudo tee -a /boot/firmware/config.txt
	@echo "Installation complete. Reboot to apply changes."

# 완전 제거
uninstall:
	@echo "Uninstalling kernel module and device tree overlay..."
	sudo rm -f /lib/modules/$(shell uname -r)/kernel/drivers/i2c/$(MODNAME).ko
	sudo rm -f $(DTBODIR)/$(DTBO_OUT)
	sudo rm -f /etc/modules-load.d/$(MODNAME).conf
	sudo sed -i '/^dtoverlay=scd4x/d' /boot/firmware/config.txt
	sudo depmod -a
	@echo "Uninstall complete."

# 재부팅
reboot:
	sudo reboot
