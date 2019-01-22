#include <stdio.h>
#include <stdint.h>
#include <windows.h>

#define JUMP_SZ 200

void taperewind(int blocksize);
void taperewind_test(int blocksize, int offset);
void initdrive(char* drivename, int blocksize);

HANDLE handle;

void geterror()
{
	DWORD err = GetLastError();
	printf("err in hex: %#010x \n", err);
	char buf[256];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 256, NULL);
	printf("%s\n", buf);
}

int main(int argc, char** argv)
{
	if (argc != 5) {
		printf("Error: specify drive name, block-size, command and and the offset on tape. Also make sure that medium is loaded to drive.\n");
		printf("Example: tapetest.exe \"\\\\.\\Tape0\" 262144 rewind 107374182400\n");
		return -1;
	}

	char* drivename = argv[1];
	int blocksize = atoi(argv[2]);
	char* cmd = argv[3];
	uint64_t datasize = _atoi64(argv[4]);

	printf("Input parameters: %s %d %s %I64d\n", drivename, blocksize, cmd, datasize);
	initdrive(drivename, blocksize);

	if (strcmp(cmd, "rewind") == 0) {
		printf("Rewinding the tape...\n");
		taperewind_test(blocksize, datasize);
	}
	CloseHandle(handle);
	return 0;
}

void initdrive(char* drivename, int blocksize)
{
	handle = CreateFile(drivename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		geterror();
		exit(-2);
	}
}

void taperewind(int blocksize)
{
	TAPE_SET_MEDIA_PARAMETERS params;
	memset(&params, 0, sizeof(params));
	params.BlockSize = (DWORD)blocksize;
	DWORD res = SetTapeParameters(handle, SET_TAPE_MEDIA_INFORMATION, &params);
	if (NO_ERROR != res) {
		printf("Failed to set blocksize.\n");
		geterror();
		CloseHandle(handle);
		exit(-3);
	}
	res = SetTapePosition(handle, TAPE_REWIND, 0, 0, 0, FALSE);
	if (NO_ERROR != res) {
		printf("Tape positioning error.\n");
		geterror();
		CloseHandle(handle);
		exit(-5);
	}
}

void taperewind_test(int blocksize, int offset) {	
	TAPE_SET_MEDIA_PARAMETERS params;
	memset(&params, 0, sizeof(params));
	params.BlockSize = (DWORD)blocksize;
	DWORD res = SetTapeParameters(handle, SET_TAPE_MEDIA_INFORMATION, &params);
	if (NO_ERROR != res) {
		printf("Failed to set blocksize.\n");
		geterror();
		CloseHandle(handle);
		exit(-3);
	}
	uint64_t jump_start = offset - JUMP_SZ;
	uint32_t sizeLow;
	uint32_t sizeHigh;
	if (jump_start > 0) {
		sizeLow = jump_start & 0xffffffff;
		sizeHigh = jump_start >> 32;
		printf("Rewinding to %I64d.\n", jump_start);
		res = SetTapePosition(handle, TAPE_LOGICAL_BLOCK, 0, sizeLow, sizeHigh, FALSE);
		if (NO_ERROR != res) {
			printf("Tape positioning error.\n");
			geterror();
			CloseHandle(handle);
			exit(-5);
		}
	}
	sizeLow = offset & 0xffffffff;
	sizeHigh = offset >> 32;
	printf("Rewinding to %I64d.\n", offset);
	res = SetTapePosition(handle, TAPE_ABSOLUTE_BLOCK, 0, sizeLow, sizeHigh, FALSE);
	if (NO_ERROR != res) {
		printf("Tape positioning error.\n");
		geterror();
		CloseHandle(handle);
		exit(-5);
	}
}