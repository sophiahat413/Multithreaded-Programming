EXEC = raytracing
EXEC1 = raytracing1
EXEC2 = raytracing2
EXEC3 = raytracing3
EXECs := \
				$(EXEC1)\
				$(EXEC2)\
				$(EXEC3)


GIT_HOOKS := .git/hooks/pre-commit
.PHONY: all
all: $(GIT_HOOKS) $(EXECs)

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

CC ?= gcc
CFLAGS = \
	-std=gnu99 -Wall -O0 -g
LDFLAGS = \
	-lm -pthread



ifeq ($(strip $(PROFILE)),1)
PROF_FLAGS = -pg
CFLAGS += $(PROF_FLAGS)
LDFLAGS += $(PROF_FLAGS)
# LDFLAGS = -pthread
endif

OBJS := \
	objects.o \
	raytracing.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


$(EXEC1): $(OBJS) main1.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(EXEC2): $(OBJS) main2.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(EXEC3): $(OBJS) main3.o
	$(CC) -o $@ $^ $(LDFLAGS)



main1.o main2.o main3.o: use-models.h
use-models.h: models.inc Makefile
	@echo '#include "models.inc"' > use-models.h
	@egrep "^(light|sphere|rectangular) " models.inc | \
	    sed -e 's/^light /append_light/g' \
	        -e 's/light[0-9]/(\&&, \&lights);/g' \
	        -e 's/^sphere /append_sphere/g' \
	        -e 's/sphere[0-9]/(\&&, \&spheres);/g' \
	        -e 's/^rectangular /append_rectangular/g' \
	        -e 's/rectangular[0-9]/(\&&, \&rectangulars);/g' \
	        -e 's/ = {//g' >> use-models.h

run:
	./$(EXEC1)
	./$(EXEC2)
	./$(EXEC3)
check: $(EXECs)
	@./$(EXEC1) && diff -u baseline.ppm out.ppm || (echo Fail; exit)
	@echo "Verified OK"
	@./$(EXEC2) && diff -u baseline.ppm out.ppm || (echo Fail; exit)
	@echo "Verified OK"
	@./$(EXEC3) && diff -u baseline.ppm out.ppm || (echo Fail; exit)
	@echo "Verified OK"

clean:
	$(RM) $(EXEC1) $(OBJS) use-models.h \
		out.ppm gmon.out
	$(RM) $(EXEC2) $(OBJS) use-models.h \
		out.ppm gmon.out
	$(RM) $(EXEC3) $(OBJS) use-models.h \
		out.ppm gmon.out
