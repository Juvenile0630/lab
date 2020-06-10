CC			= g++ 
TARGET		= DtDrServer
CFLAGS		= -g -O2 -MD -MP -std=gnu++11
LDFLAGS		= -L/usr/lib64/mysql -lmysqlclient -lpthread -lm -lrt -ldl
LIBS		= 
INCLUDE		= -I./src/server -I./src/datasink -I./src/parser -I/usr/include/ -I../DtDrLib/util/ -I../DtDrLib/mysql/
TARGETDIR	= build/target
OBJDIR		= build/objects
NOMAKEDIR	= .git% data% doc% src/bin%
CPPS		= $(shell find ../DtDrLib/ ../DtDrServer/ -name "*.cpp")
SRCS		= $(filter-out $(NOMAKEDIR), $(CPPS))
DIRS		= $(dir $(SRCS))
BINDIRS		= $(addprefix $(OBJDIR)/, $(DIRS))
OBJS		= $(addprefix $(OBJDIR)/, $(patsubst %.cpp, %.o, $(SRCS))) # patsubstは空白で区切られたものか指定の文字列に置換
DEPS		= $(OBJS:.o=.d)
TILS		= $(patsubst %.cpp, %.cpp~, $(SRCS))

ifeq "$(strip $(OBJDIR))" ""
  OBJDIR = .
endif

ifeq "$(strip $(DIRS))" ""
  OBJDIR = .
endif

default:
	@[ -d  $(OBJDIR)   ] || mkdir -p $(OBJDIR)
	@[ -d "$(BINDIRS)" ] || mkdir -p $(BINDIRS)
	@[ -d "$(TARGETDIR)" ] || mkdir -p $(TARGETDIR)
	@make all --no-print-directory

all : $(OBJS) $(TARGETDIR)/$(TARGET)

$(TARGETDIR)/$(TARGET): $(OBJS) $(LIBS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

clean:
	@rm -rf $(TARGETDIR)/$(TARGET) $(TILS) $(OBJDIR)

-include $(DEPS)

