#include <stdio.h>
#include <stdint.h>
#include <windows.h>

void taperewind_test(int blocksize, uint64_t start_offset, uint64_t stop_offset);
void initdrive(char* drivename, int blocksize);

HANDLE handle;

int main(int argc, char** argv)
{
	if (argc != 5) {
		printf("Error: specify drive name, block-size, start offset and the number of blocks to rewind.\nAlso make sure that medium is loaded to drive.\n");
		printf("Example: tape_rewind.exe \"\\\\.\\Tape0\" 262144 107374182400 200\n");
		return -1;
	}

	char* drivename = argv[1];
	int blocksize = atoi(argv[2]);
	uint64_t start_offset = _atoi64(argv[3]);
	uint64_t stop_offset = _atoi64(argv[4]);

	printf("Input parameters: %s %d %I64d %I64d\n", drivename, blocksize, start_offset, stop_offset);
	initdrive(drivename, blocksize);

	taperewind_test(blocksize, start_offset, stop_offset);
	CloseHandle(handle);
	return 0;
}

void geterror()
{
	DWORD err = GetLastError();
	printf("err in hex: %#010x \n", err);
	char buf[256];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 256, NULL);
	printf("%s\n", buf);
}

void initdrive(char* drivename, int blocksize)
{
	handle = CreateFile(drivename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		geterror();
		exit(-2);
	}
}

void taperewind_test(int blocksize, uint64_t start_offset, uint64_t stop_offset) {
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
	uint32_t dwOffsetLow;
	uint32_t dwOffsetHigh;
	if (start_offset > 0) {
		dwOffsetLow = start_offset & 0xffffffff;
		dwOffsetHigh = start_offset >> 32;
		printf("Rewinding to %I64d.\n", start_offset);
		res = SetTapePosition(handle, TAPE_LOGICAL_BLOCK, 0, dwOffsetLow, dwOffsetHigh, FALSE);		//going to the start position
		if (NO_ERROR != res) {
			printf("Tape positioning error.\n");
			geterror();
			CloseHandle(handle);
			exit(-5);
		}
	}
	dwOffsetLow = stop_offset & 0xffffffff;
	dwOffsetHigh = stop_offset >> 32;
	printf("Rewinding %I64d blocks to %I64d.\n", stop_offset, (start_offset+stop_offset));
	res = SetTapePosition(handle, TAPE_SPACE_RELATIVE_BLOCKS, 0, dwOffsetLow, dwOffsetHigh, FALSE);	//rewinding to the second position
	if (NO_ERROR != res) {
		printf("Tape positioning error.\n");
		geterror();
		CloseHandle(handle);
		exit(-5);
	}
}