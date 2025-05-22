# קומפיילר
CXX = g++
CXXFLAGS = -std=c++11 -O3 -Wall

# קבצי מקור
SRCS = MapReduceFramework.cpp Barrier.cpp test1-1_thread_1_process.cpp
OBJS = $(SRCS:.cpp=.o)

# קובץ הרצה סופי
TARGET = mapreduce_test

# כלל ברירת מחדל
all: $(TARGET)

# קישור והרכבה
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# ניקוי קבצים זמניים
clean:
	rm -f $(OBJS) $(TARGET) *~ core

# יצירת ארכיון עם כל הדרוש
tar:
	tar -cvf ex3.tar $(SRCS) Barrier.h MapReduceFramework.h Makefile README
