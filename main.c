#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

#define MAX_ABS_NAME 100

char file2Name_th[MAX_ABS_NAME];
long file1Size_th;

void *calculate_remain_time(void*);

int main(int argc, char* argv[])
{
    FILE *file1, *file2;
    char byte;
    long file1Size, i;
    pthread_t thread;
    clock_t t1, t2;
    double totalTime;

    //Abertura dos arquivos indicados nos parâmetros de entrada:
    if(argc < 3){
        printf("Insira a entrada no formato correto: cp <arquivo a ser copiado> <arquivo de copia>\nExemplo: cp video1.mpg /home/user/video2.mpg\n");
        return 1;
    }
    file1 = fopen(argv[1], "rb");
    if(!file1){
        printf("Erro: falha na abertura de %s.\n", argv[1]);
        return 1;
    }
    file2 = fopen(argv[2], "wb+");
    if(!file1){
        printf("Erro: falha na abertura de %s.\n", argv[2]);
        fclose(file1);
        return 1;
    }

    //Calculando o tamanho do arquivo a ser copiado:
    fseek(file1, 0, SEEK_END);
    file1Size = ftell(file1);
    printf("Data size of %ld bytes:\n", file1Size);

    //Cópia do arquivo:
    rewind(file1);
    file1Size_th = file1Size;
    strcpy(file2Name_th, argv[2]);
    pthread_create(&thread, NULL, calculate_remain_time, NULL);
    t1 = clock();
    for(i = 0; i < file1Size; i++){
        fread(&byte, sizeof(char), 1, file1);
        fwrite(&byte, sizeof(char), 1, file2);
    }
    pthread_cancel(thread);
    t2 = clock();
    totalTime = (double)(t2 - t1)/CLOCKS_PER_SEC;
    totalTime = ceil(totalTime);
    printf("Total time: %.0lf seconds\n", totalTime);

    fclose(file1);
    fclose(file2);
    return 0;
}

void* calculate_remain_time(void *arg){
    FILE *file2;
    long bytesFiveSec, file2Size_ant, file2Size_pos, remainBytes;
    double taxaTransf, remainTime, timeInterval;
    int time;
    short int init_flag = 1;
    clock_t start, end;

    file2 = fopen(file2Name_th, "rb+");
    if(!file2){
        printf("Erro: falha na abertura do arquivo %s pelo estimador de tempo.", file2Name_th);
        pthread_exit(NULL);
    }
    file2Size_ant = 0;
    while(1){
        //Calcular qtde de bytes transferidos em 1 segundo:
        fseek(file2, 0, SEEK_END);
        file2Size_pos = ftell(file2);
        bytesFiveSec = file2Size_pos - file2Size_ant;

        //Cálculo da taxa de transferência atual:
        if(init_flag)
            taxaTransf = ((double)bytesFiveSec)/5;
        else{
            end = clock();
            timeInterval = (double)(end - start)/CLOCKS_PER_SEC;
            taxaTransf = ((double)bytesFiveSec)/timeInterval;
            start = clock();
        }

        //Cálculo da qtde de bytes restantes:
        remainBytes = file1Size_th - file2Size_pos;

        //Cálculo e exibição da qtde de tempo restante:
        remainTime = ((double)remainBytes)/taxaTransf;
        remainTime = ceil(remainTime);
        if(init_flag)
            init_flag = 0;
        else
            printf("Time remaining: %.0lf seconds\n", remainTime);

        //Atualizando a medida do tamanho atual:
        file2Size_ant = file2Size_pos;

        //Espera de 5 segundos:
        time = clock();
        while(clock() - time < 5000){}
    }
    arg = NULL;
    return arg;
}
