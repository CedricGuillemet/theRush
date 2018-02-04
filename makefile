DEBUG=yes
SANITIZER=no

ifeq ($(SANITIZER),yes)
CC=./clang
CFLAGS=-W -faddress-sanitizer -ansi -O2 -g -DLINUX -DRETAIL -fno-strict-aliasing -I/usr/include -I/usr/local/include/bullet -IRaknet/include  -L/usr/local/lib
	LDFLAGS=
	DEBUGSUFFIX=
else
CC=g++
ifeq ($(DEBUG),yes)
	CFLAGS=-W -ansi  -g -DLINUX -DRETAIL -fno-strict-aliasing -I/usr/include -I/usr/local/include/bullet -IRaknet/include -L/usr/local/lib
	LDFLAGS=
	DEBUGSUFFIX=D
	
else
	CFLAGS=-W -Wall -ansi -O3 -DLINUX -DRETAIL -fno-strict-aliasing -I/usr/include -I/usr/local/include/bullet -IRaknet/include  -L/usr/local/lib
	LDFLAGS=
	DEBUGSUFFIX=
endif
endif
EXEC=R2
SRC= therush.cpp physics.cpp mesh.cpp menus.cpp 3rdParty/JSON_parser.c bonus.cpp net.cpp game.cpp maths.cpp content.cpp testunit.cpp gui.cpp track.cpp ZShipPhysics.cpp toolbox.cpp fx.cpp render.cpp world.cpp camera.cpp allsounds.cpp net.cpp solo.cpp ocean.cpp
#$(wildcard *.cpp)
OBJ= $(SRC:.cpp=.o)

all: $(EXEC)
ifeq ($(DEBUG),yes)
	@echo "Generating debug"
else
	@echo "Generating release"
endif

R2: $(OBJ)
	$(CC) -o ./$@ $^  -lGL -lsfml-window -lsfml-system -lsfml-audio /usr/local/lib/libBulletDynamics.a /usr/local/lib/libBulletCollision.a /usr/local/lib/libLinearMath.a /usr/local/lib/libBulletSoftBody.a Raknet/LibLinux/raknetx86_64.a ~/sanitizer/asan/asan_clang_Linux/lib/libasan64.a
#rakneti386.a
#-llibGIMPACTUtils.a -llibbulletmath.a -llibbulletmultithreaded.a -llibBulletColladaConverter.a -llibbulletsoftbody.a -llibcolladadom.a -llibconvexdecomposition.a -llibiff.a -lliblibxml.a

# ../../Libs/libbase.a ../../Libs/libgame.a -lrt -lsfml-window -lsfml-system -lGL 
R2.o: therush.cpp

%.o: %.cpp
	@$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean:
	@rm -rf *.o

mrproper: clean
	@rm -rf $(EXEC)
