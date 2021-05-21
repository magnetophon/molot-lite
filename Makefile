subdirs = Molot_Mono_Lite Molot_Stereo_Lite

all:
	$(foreach dir,$(subdirs),make -C $(dir) all;)

clean:
	$(foreach dir,$(subdirs),make -C $(dir) clean;)

.PHONY: all clean
