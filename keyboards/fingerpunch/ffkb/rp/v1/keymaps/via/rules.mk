VIA_ENABLE = yes

NKRO_ENABLE = yes
COMBO_ENABLE = yes
CONSOLE_ENABLE = yes
EXTRAKEY_ENABLE = yes
LAYER_LOCK_ENABLE = yes
CAPS_WORD_ENABLE = yes
KEY_OVERRIDE_ENABLE = yes
OS_DETECTION_ENABLE = yes

FP_TRACKBALL_ENABLE=yes
FP_EC11=yes
AUDIO_ENABLE=yes

#SRC += features/sentence_case.c
#SRC += features/select_word.c

# MACCEL
#MACCEL_VIA_ENABLE = no
#SRC += ./maccel/maccel.c
#ifeq ($(strip $(VIA_ENABLE)), yes)
#	ifeq ($(strip $(MACCEL_VIA_ENABLE)), yes)
#		SRC += ./maccel/maccel_via.c
#	endif
#endif
#OPT_DEFS += -DMACCEL_ENABLE
