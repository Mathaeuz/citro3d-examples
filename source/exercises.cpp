#include "exercises.h"
#include "ex/00.h"
#include "ex/01.h"
#include "ex/02.h"
#include "ex/03.h"

const int getExerciseCount() { return 4; }

const char *getExerciseName(int id)
{
    switch (id)
    {
    default:
        return "TrisImmediate - Composite";
    case 1:
        return "Elements - Normal from position";
    case 2:
        return "Elements - Textured";
    case 3:
        return "Imgui portlibs";
    }
}

void runExercise(int id)
{
    switch (id)
    {
    default:
        run00();
        return;
    case 1:
        run01();
        return;
    case 2:
        run02();
        return;
    case 3:
        run03();
        return;
    }
}