#include "graphics.h"

int start() {
    VBEInfoBlock* VBE = (VBEInfoBlock*) VBEInfoAddress;

	mx = VBE->x_resolution / 2;
	my = VBE->y_resolution / 2;
	// String literals cannot be more than 61 characters.
	char str1[] = "Welcome to SaphireOS!\n\nText rendered by custom library.";
	char *p = str1;

	char characterBuffer[1000] = "\0";
	char* characterBufferPointer = characterBuffer;
	int characterBufferLength = 0;

	base = (unsigned int) &isr1;
	base12 = (unsigned int) &isr12;

	InitialiseMouse();
	InitialiseIDT();

	tasks[TasksLength].priority = 0;
	tasks[TasksLength].function = &ClearScreenTask;
	TasksLength++;

	// tasks[TasksLength].priority = 0;
	// tasks[TasksLength].taskId = TasksLength;
	// tasks[TasksLength].function = &TestGraphicalElementsTask;
	// iparams[TasksLength * task_params_length + 0] = 50;
	// iparams[TasksLength * task_params_length + 1] = 50;
	// iparams[TasksLength * task_params_length + 2] = 300;
	// iparams[TasksLength * task_params_length + 3] = 300;
	// iparams[TasksLength * task_params_length + 4] = 0;
	// iparams[TasksLength * task_params_length + 5] = 0;
	// iparams[TasksLength * task_params_length + 6] = 0;
	// TasksLength++;

	tasks[TasksLength].priority = 0;
	tasks[TasksLength].function = &TaskbarTask;
	tasks[TasksLength].taskId = TasksLength;
	iparams[TasksLength * task_params_length + 0] = 0;
	iparams[TasksLength * task_params_length + 1] = 0;
	iparams[TasksLength * task_params_length + 2] = VBE->x_resolution;
	iparams[TasksLength * task_params_length + 3] = 40;
	iparams[TasksLength * task_params_length + 4] = 1;
	TasksLength++;

	tasks[TasksLength].priority = 0;
	tasks[TasksLength].function = &HandleKeyboardTask;
	TasksLength++;

	tasks[TasksLength].priority = 5;
	tasks[TasksLength].function = &DrawMouseTask;
	TasksLength++;

	// TasksLength++;


	while(1) {
		ProcessTasks();

		Flush();
	}
}