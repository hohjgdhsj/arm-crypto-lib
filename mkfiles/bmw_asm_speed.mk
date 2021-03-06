# Makefile for BlueMidnightWish
ALGO_NAME := BMW_ASM_SPEED

# comment out the following line for removement of BlueMidnightWish from the build process
HASHES += $(ALGO_NAME)

$(ALGO_NAME)_DIR      := bmw/
$(ALGO_NAME)_INCDIR   := memxor/ hfal/
$(ALGO_NAME)_OBJ      := bmw_small_speed_cstub.o bmw_small_speed_asm_f0.o bmw_large_speed.o memxor.o
$(ALGO_NAME)_TESTBIN := main-bmw-test.o hfal_bmw_small.o hfal_bmw_large.o $(CLI_STD) $(HFAL_STD)
$(ALGO_NAME)_NESSIE_TEST      := test nessie
$(ALGO_NAME)_PERFORMANCE_TEST := performance

