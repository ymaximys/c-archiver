#include <stdio.h>
#include <string.h>

#define MAX_FILES_ARCHIVE 10
#define MAX_FILE_NAME 64 

typedef struct {
    char file[MAX_FILE_NAME]; //file name
    size_t size; //file size
    size_t pos; //position in archive
} ArchiveFile;	

typedef struct {
    ArchiveFile archivedFiles[MAX_FILES_ARCHIVE]; //archived files metadata
    int files; //amount of files to archive   
} Metadata;

typedef struct {
    int is_arch; //flag identify that it's archave operation
    int is_unarch; //flag identify that it's unacrive opration
    char arch_file[MAX_FILE_NAME]; //path to archive file
    char files_to_arch[MAX_FILES_ARCHIVE][MAX_FILE_NAME]; //array of files to archive
    int files_amount; // amount of files to archive
} Parameters;

size_t fileSize(char *fileName);

int archive(char *archiveName, Metadata *header);
int unarchive(Parameters *par);

int createHeader(Parameters *par, Metadata *header);
int parseInputParams(int argc, char **argv, Parameters *par);
int validateInputParams(Parameters *par);


int main(int argc, char **argv) {
    
    for (int i = 0; i < argc; i++) {
        printf("[%d] parameter - %s ", i, *(argv + i));
	    printf("\n");
    }

    Parameters params;
    params.is_arch = 0;
    params.is_unarch = 0;
    params.files_amount = 0;

    if (parseInputParams(argc, argv, &params)) {
        fprintf(stderr, "Error ocurred during parsing input parameters\n");	    
        return 1;
    }	 
    
    if (validateInputParams(&params)) {
        return 1;
    }

    if (params.is_arch) {
        Metadata header;
        header.files = 0;
        if (createHeader(&params, &header)) {
            return 1;    
        }	    
    
        printf("header %s archived file is %d\n", params.arch_file, header.files);
        for (int i = 0; i < header.files; i ++) {
	        ArchiveFile aFile = header.archivedFiles[i];    
            printf("File to archive %s, size %ld and position in archive %ld \n", aFile.file, aFile.size, aFile.pos);	
        }	    

        archive(params.arch_file, &header);
    } else if (params.is_unarch) {
        unarchive(&params);
    }
    return 0;
}

int validateInputParams(Parameters *par) {
    if (par->is_arch && par->is_unarch) {
        fprintf(stderr, "Parameters -a and -una cannot be provided simultaneously");
        return 1;
    }

    if (!par->is_arch && !par->is_unarch) {
        fprintf(stderr, "One of the parameter -a or -una should be provided");
        return 1;
    }

    if(par->is_arch && par->files_amount == 0) {
        fprintf(stderr, "To perform archive operation atleast obe file should be provided, please use -l parameter");
        return 1;
    }
    return 0;
}

int parseInputParams(int argc, char **argv, Parameters *par) {
    for (int i = 1; i < argc; i++) {
        char *param = *(argv + i);
        if (strcmp(param, "-a") == 0) { //parameter that indicate archive operation
	        if (i == (argc - 1)) {
	            fprintf(stderr, "Name of an archive should be provided after -a arg\n");
                return 1;		
            }
            if ('-' == (*(argv + i + 1))[0]) { // check if next arg is a command
                fprintf(stderr, "Name of an archive should be provided after -a arg\n");
                return 1;
            }

            i++; //increament to get arch name;
	        par->is_arch = 1;
            strncpy(par->arch_file, *(argv + i), MAX_FILE_NAME);
            par->arch_file[MAX_FILE_NAME - 1] = '\0'; //manually set null terminator
	    } else if (strcmp(param, "-una") == 0) { //indicate unarchive operation
	        if (i == (argc - 1)) {
                fprintf(stderr, "Name of file to unarchive should be provided after -una arg\n");
                return 1;		
            }	
            if ('-' == (*(argv + i + 1))[0]) { // check if next arg is a command
                fprintf(stderr, "Name of file to unarchive should be provided after -una arg\n");
                return 1;
            }

	        i++; //inc to get next arg which should be unarchive file
	        par->is_unarch = 1;
            strncpy(par->arch_file, *(argv + i), MAX_FILE_NAME);
            par->arch_file[MAX_FILE_NAME - 1] = '\0'; //manually set null terminator
	    } else if (strcmp(param, "-l") == 0) { //indicate list of the files to archive
	        if (i == (argc - 1)) {
                fprintf(stderr, "Files to archive should be provided after -l arg\n");
                return 1;
	        }
            if ('-' == (*(argv + i + 1))[0]) { // check if next arg is a command
                fprintf(stderr, "Files to archive should be provided after -l arg\n");
                return 1;
            }

            i++; //switch to next arg to read files
            for(;i < argc; i++) {
                param = *(argv + i);
                if ('-' == param[0]) { // check if next arg is not command
                    i--; //rollback counter
                    fprintf(stderr, "No files to archive were provided after -l arg\n");
                    break;
                }
                strncpy(par->files_to_arch[par->files_amount], param, MAX_FILE_NAME);
                par->files_to_arch[par->files_amount][MAX_FILE_NAME - 1] = '\0'; //manually set null terminator
                par->files_amount++;
            }		    
	    }	
    }	
    return 0;    
}

int createHeader(Parameters *par, Metadata *header) {
    size_t position = sizeof(Metadata); //position of the first file should be after metadata 
    
    for (int i = 0; i < par->files_amount; i++) {
        char *fileName = par->files_to_arch[i];
	    size_t file_size;
	    if ((file_size = fileSize(fileName)) == 0) {
	        fprintf(stderr, "File not found %s, terminating program\n", fileName);
            return 1;		
        }

	    ArchiveFile* aFile = &(header->archivedFiles[header->files]);
	    strncpy(aFile->file, fileName, MAX_FILE_NAME);
	    aFile->pos = position;
	    aFile->size = file_size;
	    position += file_size; //calculate position for the next file in archive
	    header->files++;
    }

	if (header->files >= MAX_FILES_ARCHIVE) {
	    fprintf(stderr, "Archivator can only archive %d files, other will be skiped", MAX_FILES_ARCHIVE);
    }		    
		
    return 0;
}

int unarchive(Parameters *par) {
    FILE *archive;
    if((archive = fopen(par->arch_file, "rb")) == NULL) {
        perror("Error to write to archive");    
        return 1;
    }

    int seek_result = fseek(archive, 0, SEEK_SET);
    if (seek_result != 0) {
        perror("Error seeking file");
        fclose(archive);
        return 1;
    }
    
    Metadata metadata;
    size_t bytes = fread(&metadata, sizeof(Metadata), 1, archive);
    if (bytes == 0) {
        fprintf(stderr, "Problem to read file or it empty");
        fclose(archive);
        return 1;
    }

    printf("Files to extract %d\n", metadata.files);
    for (int i = 0; i < metadata.files; i++) {
        ArchiveFile arch_file_meta = metadata.archivedFiles[i];
        FILE *file_to_unarch;
        if ((file_to_unarch = fopen(arch_file_meta.file, "wb")) == NULL) {
	        perror("Error to write to archive");  
            fclose(archive);
            return 1;
        }

        size_t buffer_size = 4096;
        char buffer[buffer_size];
        size_t read_bytes = 0;
        size_t bytes_left_read = arch_file_meta.size; //file size to read;

        do {
            size_t bytes_to_read = buffer_size < bytes_left_read ? buffer_size : bytes_left_read;
            read_bytes = fread(buffer, 1, bytes_to_read, archive);
            fwrite(buffer, 1, read_bytes, file_to_unarch);
            bytes_left_read -= read_bytes;
        } while(bytes_left_read > 0);

        fclose(file_to_unarch); 
    }

    fclose(archive);
    return 0;
}

int archive(char *archiveName, Metadata *metadata) {
    FILE *archive;
    if ((archive = fopen(archiveName, "wb")) == NULL) {
	    perror("Error to write to archive");    
        return 1;
    }  
    
    //first step to write metadata to archive
    size_t bytes = fwrite(metadata, sizeof(Metadata), 1, archive);
    if (bytes == 0) {
        fprintf(stderr, "No data was writen to archive %ld", bytes);
	    fclose(archive);
	    return 1;
    }	    

    for (int i = 0; i < metadata->files; i++) {
        FILE *file;
        if ((file = fopen(metadata->archivedFiles[i].file, "rb")) == NULL) {
	        perror("Error to read file");
	        fclose(archive);
	        return 1;
	    }
        
	    char buffer[4096];
	    size_t read_bytes = 0;
        size_t total_bytes = 0;
        while ((read_bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
	        fwrite(buffer, 1, read_bytes, archive);
	        total_bytes += read_bytes;
	    }	
	    printf("File %s was archived, transfer size %ld, file size %ld\n", metadata->archivedFiles[i].file, total_bytes, metadata->archivedFiles[i].size);
        fclose(file);	
    }	    

    fclose(archive);
    return 0;
}	

size_t fileSize(char *fileName) {
    FILE *file;
    
    size_t file_size = 0;

    if ((file = fopen(fileName, "rb"))) { //open file in byte read mode
	
        if (fseek(file, 0L, SEEK_END) == 0) { //move to the end of the file
	        file_size = ftell(file); //get position in file 	
	    }

        rewind(file);	
        fclose(file);
	    return file_size;
    }
    return file_size;
}