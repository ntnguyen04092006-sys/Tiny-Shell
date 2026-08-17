#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>


//CTDL
typedef struct ProcessControl {
    char cmdName[256];      
    DWORD dwProcessId;      
    int status;             
    PROCESS_INFORMATION pi; 
}ProcessControl;

ProcessControl plist[100];
int pcount = 0;


void trim(char* s) {
    if (s == NULL) return;
    char* start = s;
    while (isspace((unsigned char)*start)) start++;
    if (*start == '\0') { 
        s[0] = '\0';
        return;
    }
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    size_t len = end - start + 1;
    memmove(s, start, len);
    s[len] = '\0';
}

char shellPaths[10][256]; 
int pathCount = 0;

PROCESS_INFORMATION* currentfp = NULL;

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    switch (fdwCtrlType) {
        case CTRL_C_EVENT:
            if (currentfp != NULL) {
                printf("\n[Shell] Dang ngat tien trinh Foreground (PID: %d)...\n", currentfp->dwProcessId);
                TerminateProcess(currentfp->hProcess, 0);
                return TRUE; 
            }
            printf("\nMyShell> ");
            return TRUE;
    	default:
        	return FALSE;
    }
}

void listPath() {
    printf("Cac duong dan hien tai trong PATH:\n");
    int i;
    for (i = 0; i < pathCount; i++) {
        printf("%d. %s\n", i + 1, shellPaths[i]);
    }
}
void addPath(char* newPath) {
    if (pathCount < 10) {
        strcpy(shellPaths[pathCount], newPath);
        pathCount++;
        printf("Da them duong dan: %s\n", newPath);
    } else {
        printf("Loi: Bo nho PATH da day!\n");
    }
}


bool isBatchFile(const char* name) {
    size_t len = strlen(name);
    return (len >= 4 && _stricmp(name + len - 4, ".bat") == 0);
}

void createNewProcess(char* input) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);
	
	trim(input);
	bool background = FALSE;
	int len = strlen(input);
	if(len > 0 && input[len-1] == '&'){
		background = TRUE;
		input[len-1] = '\0';
		trim(input);
	}
	char commandLine[1024];
    if (isBatchFile(input)) {
        sprintf(commandLine, "cmd.exe /c \"%s\"", input);
    } else {
        strcpy(commandLine, input);
    }
	
	if(CreateProcess(NULL,commandLine,NULL,NULL,FALSE, CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi)){
		if(background != TRUE){
			currentfp = &pi;
			WaitForSingleObject(pi.hProcess,INFINITE);
		}
		plist[pcount].pi = pi;
		strcpy(plist[pcount].cmdName, input);
		plist[pcount].dwProcessId = pi.dwProcessId;
		plist[pcount].status = 1;
		pcount++;
		return;
	}
	char fullPath[512];
	int i;
    for (i = 0; i < pathCount; i++) {
        strcpy(fullPath, shellPaths[i]);
        strcat(fullPath, "\\");
        strcat(fullPath, input);
        char pathCommandLine[1024];
        if (isBatchFile(fullPath)) {
	        sprintf(pathCommandLine, "cmd.exe /c \"%s\"", fullPath);
	    } else {
	        strcpy(pathCommandLine, fullPath);
	    }
        if (CreateProcess(NULL, pathCommandLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
        	if(background != TRUE){
				currentfp = &pi;
				WaitForSingleObject(pi.hProcess,INFINITE);
			}
			plist[pcount].pi = pi;
			strcpy(plist[pcount].cmdName, input);
			plist[pcount].dwProcessId = pi.dwProcessId;
			plist[pcount].status = 1;
			pcount++;
            return;
        }
    }
    printf("Loi: Khong the thuc thi '%s'. Kiem tra lai ten hoac PATH.\n", input);
    return;
}

void printList() {
    printf("\n%-5s %-25s %-10s %-15s\n", "STT", "Command Name", "PID", "Status");
    printf("------------------------------------------------------------\n");
	int i;
    for (i = 0; i < pcount; i++) {
        DWORD exitCode;
        if (plist[i].status != 0 && GetExitCodeProcess(plist[i].pi.hProcess, &exitCode)) {
            if (exitCode != STILL_ACTIVE) {
                plist[i].status = 0; 
                CloseHandle(plist[i].pi.hProcess); 
                CloseHandle(plist[i].pi.hThread);
            }
            else if (plist[i].status != 2) {
                plist[i].status = 1; 
            }
        }
        printf("%-5d %-25s %-10d %-15s\n", i + 1, plist[i].cmdName, plist[i].dwProcessId, (plist[i].status == 1) ? "Running" : (plist[i].status == 2 ? "Suspended" : "Stopped"));
    }
    printf("------------------------------------------------------------\n\n");
}


void killProcess(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        printf("Loi: Khong the mo tien trinh PID %d (co the da dong hoac sai PID).\n", pid);
        return;
    }
    if (TerminateProcess(hProcess, 0)) {
        printf("Da gui lenh terminate toi PID %d thanh cong.\n", pid);
    } else {
        printf("Loi: Khong the kill PID %d. Error code: %d\n", pid, GetLastError());
    }
    CloseHandle(hProcess);
}


void stopProcess(DWORD pid) {
	int i;
    for (i = 0; i < pcount; i++) {
        if (plist[i].dwProcessId == pid) {
            if (SuspendThread(plist[i].pi.hThread) != (DWORD)-1) {
                printf("Da tam dung (Stop) tien trinh PID %d.\n", pid);
                plist[i].status = 2; 
                return;
            }
        }
    }
    printf("Loi: Khong tim thay PID %d trong danh sach quan ly.\n", pid);
}

void resumeProcess(DWORD pid) {
	int i;
    for (i = 0; i < pcount; i++) {
        if (plist[i].dwProcessId == pid) {
            if (ResumeThread(plist[i].pi.hThread) != (DWORD)-1) {
                printf("Da tiep tuc (Resume) tien trinh PID %d.\n", pid);
                plist[i].status = 1; 
                return;
            }
        }
    }
    printf("Loi: Khong tim thay PID %d hoac tien trinh khong chay.\n", pid);
}


int main(){
	printf("--- NTN OS Shell Management ---\n");
	
	if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
        printf("Loi: Khong the thiet lap Ctrl Handler.\n");
        return 1;
    }
	
	char input[256];
	while(1){
	    printf("NTNShell> \n"); 
	    if (fgets(input, sizeof(input), stdin) == NULL)	break;
	    input[strcspn(input, "\n")] = '\0';
	    
	    if (strcmp(input, "exit") == 0){
	    	break;
		}
		
		else if (strcmp(input, "help") == 0) {
            printf("Cac lenh dac biet: help, date, time, dir, list, path, addpath\n");
        }
        
	    else if (strcmp(input, "list") == 0) {
            printList(); 		
        }
        
        else if (strcmp(input, "date") == 0 || strcmp(input, "time") == 0) {
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            if (strcmp(input, "date") == 0)
                printf("Ngay hien tai: %02d/%02d/%d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
            else
                printf("Gio hien tai: %02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
        }
        
		else if (strncmp(input, "addpath ", 8) == 0) {
		    addPath(input + 8); 
		}
		else if (strcmp(input, "path") == 0) {
		    listPath();
		}
        
        else if (strncmp(input, "kill ", 5) == 0) {
		    DWORD pid = (DWORD)atoi(input + 5);
		    killProcess(pid);
		}
		else if (strncmp(input, "stop ", 5) == 0) {
		    DWORD pid = (DWORD)atoi(input + 5);
		    stopProcess(pid);
		} 
		else if (strncmp(input, "resume ", 7) == 0) {
		    DWORD pid = (DWORD)atoi(input + 7);
		    resumeProcess(pid);
		}
        
        
		else{
        	createNewProcess(input);
		}
		if (strlen(input) == 0) continue;
		
		
		
	}

	return 0;
}
