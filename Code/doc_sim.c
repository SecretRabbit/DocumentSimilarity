/**
@file
 *Dogru calismasi icin exe dosyasi file dosyalari ile ayni konumda olmalidir.
 *Bu program, input olarak verilen txt dosyalarindan shinglelar olusturularak,
 *dosyalarin shingle sayisi bilgilerini bir tabloda yazdirir.
 *jaccard yontemiyle ve minhashing sonucu olusan signature similarity yontemiyle
 *dokumanlarin benzerlik oranlarini tablo halinde ekrana yazdirir.
 *Benzerlik tablolarinda ise verilen bir threshold degeri asan benzerlikleri isaretleyerek(sari renkle yazdirir)
 *tum bunlara ek olarak benzer dosya ikililerini bir tabloda yazdirir.
 * Programda 1 ile devam edilir 0 ile cikis yapilmaktadir.
 * Oncelikle dosya sayisi ve isimleri girilmelidir.
 * Daha sonrasinda ise harf bazindaki shingle lar icin k degeri kullanicidan alinmaktadir.
 * Son olarak ise kullanicidan threshold degeri 0.0 ile 1.0 arasinda bir deger olarak alinmaktadir.
 * Tum inputlar girildikten sonra program aciklamasinda verilen tablolar ekrana yazdirilmaktadir.
 * Tavsiye olarak tablolarin ekrana sigmasi ve anlasilir gozukmeleri icin konsol ekranindaki font boyutu 14 olarak
 * ayarlanmali ve konsol ekrani tam ekran olarak kullanilmalidir.


@author

Name 			 :		Muhammed Yasin SAGLAM
Student No		 :	 	15011804
Date 			 :		31/12/2017
E-Mail			 :		myasinsaglam1907@gmail.com
Compiler Used	 :		GCC
IDE				 :		DEV-C++(Version 5.11)
Operating System :		Windows 10 educational edition
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#define NAME_LEN 30
#define HASH_SIZE 100

typedef struct{
    char **shingles; //kiyas yapabilmek icin tum shingle lar karakter olarak saklaniyor
    int k_val; //Shingle larin k degeri tutuluyor
    int **matrix; //shingle ve file matrisi tutuluyor
    float **jaccard;//jaccard similarity oranlari tutuluyor
    float **minhash;//minhash signature similarity oranlari tutuluyor
    int file_num; //kac adet dosya bulundugu bilgisini tutar --matrix sutun sayisi
    int shingle_count; //kac adet shingle icerdigi bilgisi tutuluyor--matrix satir sayisi
    float threshold;//threshold degeri
}SHINGLE;

/**
	@param shgl			Shingle struct pointer
*/
void initialize(SHINGLE *shgl){
    shgl->shingle_count=0; //toplsm shingle sayisi sifirlaniyor
    //Matrix ve shinglelari tutacak diziler reallocate edilebilmeleri icin ilk allocationlari yapiliyor.
    shgl->matrix=(int**)malloc(sizeof(int*));
    shgl->matrix[0]=(int*)calloc((size_t)shgl->file_num,sizeof(int));
    shgl->shingles=(char**)malloc(sizeof(char*));
    shgl->shingles[0]=(char*)malloc(sizeof(char)*shgl->k_val);
}

/**
 * Dosyalari okuyarak shinglelarini cikartan ve her bir dosyanin
 * icerisinde bulunan shingle lari struct icerisindeki matrix adli
 * matriste saklayan fonksiyon
	@param fp			shingle olusturulacak dosyanin pointeri
    @param file_id      id of file
    @param shgl        Shingle struct pointer
*/
void createShingles(FILE *fp,int file_id,SHINGLE *shgl){
    int i,c,j;
    int s_index=shgl->shingle_count;
    //int total_shingle=0;
    char *temp_shingle=(char*)calloc(shgl->k_val,sizeof(char));

    int next_start=0;
    int control=1;

    next_start=ftell(fp);
    do{
        fseek(fp,next_start-1,SEEK_SET);
        i=0;
        while(control && i<shgl->k_val){
            c=fgetc(fp); //1 karakter okunuyor
            if((char)c==EOF){ //dosya sonu ise cikis
                control=0;
            }
            else{ //degilse
                if(isalpha(c)){ //alfabetik karakterse
                    temp_shingle[i]=(char)c; //tempe yaz
                    if(i==1) //eger tempin 1. elemanina yaziyosa 1 sonraki oradan baslayacak
                        next_start=ftell(fp); //dosyadaki yerini tut
                    i++; //indisi 1 artir
                }
                if((char)c==' '){ //eger bosluk gelmisse
                    if(i>=1 && temp_shingle[i-1]!=' '){ //ilk elemandan sonraki bir eleman icin bosluksa bir onceki bosluk degilse bosluk yaz
                        temp_shingle[i]=(char)c;
                        if(i==1)
                            next_start=ftell(fp);
                        i++;
                    }
                    else if(i==0){ //ilk eleman bosluksa direk yaz
                        temp_shingle[i]=(char)c;
                        i++;
                    }
                }
            }
        }
        if(control){//dosya sonundan dolayi cikmamissa tamamen k kadarlik bir shingle alinmis demektir
            j=0;
            strlwr(temp_shingle); //shingle i kucuk harfe cevir
            while(j<shgl->shingle_count && strcmp(shgl->shingles[j],temp_shingle)!=0){ //eger shingle eklenmisse kc nolu indiste
                j++;
            }
            if(j==shgl->shingle_count){//yeni eklenecek demektir
                shgl->matrix=(int**)realloc(shgl->matrix, sizeof(int*)*(s_index+1)); //matrisi genislet
                shgl->matrix[s_index]=(int*)calloc((size_t)shgl->file_num,sizeof(int));
                shgl->matrix[s_index][file_id]=1; //ilgili degeri 1 yap
                shgl->shingles=(char**)realloc(shgl->shingles, sizeof(char*)*(s_index+1)); //shigle listi genislet
                shgl->shingles[s_index]=(char*)malloc(sizeof(char)*(shgl->k_val));
                strcpy(shgl->shingles[s_index],temp_shingle); //shingle i yaz
                //printf("\nshingle %d : %s ------ %s",s_index+1,temp_shingle,shgl->shingles[s_index]);
                shgl->shingle_count++;
                s_index++;
                temp_shingle=(char*)malloc(shgl->k_val*sizeof(char));
                // total_shingle++; // dosyadaki toplam shingle sayisi
            }
            else{//daha onceden varsa matrisin ilgili gozunu 1 yap
                shgl->matrix[j][file_id]=1;
                temp_shingle=(char*)malloc(shgl->k_val*sizeof(char));
            }
        }
        //kelime alindi kontrol edilecek
    }while (control);
    //system("PAUSE");
    //printf("\nTotal shingle of file is : %d ",total_shingle);
}

/**
 * Her bir dosya icin tekrarsiz shingle sayisini ve toplam shingle sayisini
 * tablo olarak ekrana yazan fonksiyon
    @param shgl        Shingle struct pointer
    @param files      File names
*/
void print_shingle(SHINGLE *shgl,char **files){
    system("CLS");
    int i,j;
    int *shgl_per_file=(int*)calloc((size_t)shgl->file_num,sizeof(int));
    for(i=0;i<shgl->shingle_count;i++){
        //printf("\n%-5s -->",shgl->shingles[i]);
        for(j=0;j<shgl->file_num;j++){
            //printf(" %d ",shgl->matrix[i][j]);
            if(shgl->matrix[i][j]==1)
                shgl_per_file[j]++;
        }
    }
    for(i=0;i<shgl->file_num;i++){
        printf("\nFile : %-5s --> Shingle Count: %d ",files[i],shgl_per_file[i]);
    }
    printf("\nTotal shingle of all files for k=%d : %d",shgl->k_val,shgl->shingle_count);
    printf("\n\n\n");
    for(i=0;i<100;i++){
        printf("-");
    }
    printf("\n");
    free(shgl_per_file);
}

/**
 * Struct icindeki jaccard matrisinde bulunan dosyalarin benzerligini ve verilen thresholda gore,
 * benzer dosya ikililerini ekrana tablo olarak yazan fonksiyon
    @param shgl        Shingle struct pointer
    @param files      File names
*/
void printJaccard(SHINGLE *shgl,char **files){
    int i,j;
    //tablo ekrana yazdiriliyor...
    printf("\t");
    for(i=0;i<shgl->file_num;i++){
        printf("%s\t",files[i]);
    }
    printf("\n");
    for(i=0;i<shgl->file_num;i++){
        printf("%s\t",files[i]);
        for(j=0;j<shgl->file_num;j++){
            if(shgl->jaccard[i][j]>shgl->threshold){ //thresholdu gecen satirlar sari yazdiriliyor..
                printf("\033[01;33m");
                printf("%.2f\t",shgl->jaccard[i][j]);
                printf("\033[0m");
            }
            else if(i==j){ //diyagonal kirmizi yazdiriliyor..
                shgl->jaccard[i][j]=1;
                printf("\033[1;31m");
                printf("%.2f\t",shgl->jaccard[i][j]);
                printf("\033[0m");
            }
            else{
                printf("%.2f\t",shgl->jaccard[i][j]);
            }
        }
        printf("\n");
    }
    //benzer ikililer yazdiriliyor
    printf("\nSIMILIAR DOCUMENT PAIRS ACCORDING TO JACCARD SIMILARITY");
    for(i=0;i<shgl->file_num;i++) {
        printf("\n%s-->>\t", files[i]);
        for (j = 0; j < shgl->file_num; j++) {
            if (shgl->jaccard[i][j] > shgl->threshold && j!=i) { //thresholdu gecen satirlar sari yazdiriliyor..
                printf("\033[01;36m");
                printf("(%s) ", files[j]);
                printf("\033[0m");
            }
        }
    }
    printf("\n\n\n");
    for(i=0;i<100;i++){
        printf("-");
    }
    printf("\n");
}

/**
 * Jaccard benzerligini hesaplayarak oranlari struct icerisindeki
 * jaccard matrisine yazan fonksiyon
    @param shgl        Shingle struct pointer
    @param files      File names
*/
void calculate_jaccard(SHINGLE *shgl,char **files){
    int total_1=0;
    int total_2=0;
    int intersect=0;
    int union_all;
    int i,j,k;
    //all permutation loop n*(n-1)/2
    shgl->jaccard=(float**)malloc(shgl->file_num*sizeof(float*)); //jaccard matrisi olusturuldu
    for(i=0;i<shgl->file_num;i++){
        shgl->jaccard[i]=(float*)calloc((size_t)shgl->file_num,sizeof(float));
    }
    printf("\n\nK VALUE : %d THRESHOLD : %.2f\nJACCARD SIMILARITIES OF ALL DOCUMENT COMBINATIONS\n",shgl->k_val,shgl->threshold);
    for(i=0;i<shgl->file_num;i++){
        for(j=i+1;j<shgl->file_num;j++){
            //printf("\nFile %d (%s) - File %d (%s) : ",i+1,files[i],j+1,files[j]);
            for(k=0;k<shgl->shingle_count;k++){
                //printf("\n%d %d",shgl->matrix[k][i],shgl->matrix[k][j]);
                if(shgl->matrix[k][i]==1)
                    total_1++;
                if(shgl->matrix[k][j]==1)
                    total_2++;
                if(shgl->matrix[k][i]==1 && shgl->matrix[k][j]==1)
                    intersect++;
            }
            union_all=total_1+total_2-intersect;
            shgl->jaccard[i][j]=((float)intersect/(float)union_all);
            shgl->jaccard[j][i]=shgl->jaccard[i][j];
            //printf("%.2f ",shgl->jaccard[i][j]);
            total_1=total_2=intersect=0;
        }
    }
    //Sonuclar ekrana yazdiriliyor
    printJaccard(shgl,files);

}

/**
 * Hash degeri ureten hash fonksiyonu
    @param a   random value
    @param x   0 to hash_Size
    @param m   Shingle count
*/
int hash(int a,int x, int m){
    return ((a*x+1)%m);
}

/**
 * Signature benzerligine gore hesaplanan dosyalarin,
 * Benzerlik oranlari ve benzer ikililer gibi sonuclari ekrana yazdiran fonksiyon
 * @param shgl      Shingle struct pointer
 * @param files     Filenames
 */
void printSignature(SHINGLE *shgl,char **files){
    int i,j;
    //Hesaplanan signature similarity oranlari tablo olarak ekrana yazdiriliyor
    printf("\t");
    for(i=0;i<shgl->file_num;i++){
        printf("%s\t",files[i]);
    }
    printf("\n");
    for(i=0;i<shgl->file_num;i++){
        printf("%s\t",files[i]);
        for(j=0;j<shgl->file_num;j++){
            if(shgl->minhash[i][j]>shgl->threshold){
                printf("\033[01;33m");
                printf("%.2f\t",shgl->minhash[i][j]);
                printf("\033[0m");
            }
            else if(i==j){
                shgl->minhash[i][j]=1;
                printf("\033[1;31m");
                printf("%.2f\t",shgl->minhash[i][j]);
                printf("\033[0m");
            }
            else{
                printf("%.2f\t",shgl->minhash[i][j]);
            }
        }
        printf("\n");
    }
    //benzer ikililer yazdiriliyor
    printf("\nSIMILIAR DOCUMENT PAIRS ACCORDING TO SIGNATURE SIMILARITY");
    for(i=0;i<shgl->file_num;i++) {
        printf("\n%s-->>\t", files[i]);
        for (j = 0; j < shgl->file_num; j++) {
            if (shgl->minhash[i][j] > shgl->threshold && j!=i) { //thresholdu gecen satirlar sari yazdiriliyor..
                printf("\033[01;36m");
                printf("(%s) ", files[j]);
                printf("\033[0m");
            }
        }
    }
    printf("\n\n\n");
    for(i=0;i<100;i++){
        printf("-");
    }
    printf("\n");
}
/**
 * Imza matrisine gore benzerlik oranlarini struct icerisinde minhash matrisine yazan fonksiyon
 * @param shgl
 * @param files
 */
void calculate_minhash(SHINGLE *shgl,char **files){
    int intersect=0; //kesisim sayisi
    int i,j,k; //indis degiskenleri
    int temp;
    int a[HASH_SIZE];
    int rand_val; //random deger
    //imza matrisi olusturuluyor
    int **signature=(int**)malloc(sizeof(int*)*HASH_SIZE);
    for(i=0;i<HASH_SIZE;i++){
        signature[i]=(int*)malloc(sizeof(int)*shgl->file_num);
    }
    //ilk deger atamasi olarak her bir goze sonsuz niteliginde hicbir zaman alamayacagi bir deger olan shingle sayisi ataniyor.
    for(i=0;i<HASH_SIZE;i++){
        for(j=0;j<shgl->file_num;j++){
            signature[i][j]=shgl->shingle_count;
        }
    }
    //hash matrisi olusturuluyor icinde hashlenmis degerleri tutar
    int **hash_mtr=(int**)malloc(sizeof(int*)*shgl->shingle_count);
    for(i=0;i<shgl->shingle_count;i++){
        hash_mtr[i]=(int*)calloc((size_t)HASH_SIZE,sizeof(int));
    }
    srand(time(NULL));
    rand_val=rand()%shgl->shingle_count-1;
    a[0]=rand_val;
    //printf("%-3d ",rand_val);
    for(i=1;i<HASH_SIZE;i++){ //random tekrarsiz a degerleri icin dizi olusturuluyor
        rand_val=rand()%shgl->shingle_count-1; //rastgele bir sayi uret
        j=0; //0.sayidan itibaren
        while(j<i){ //en son uretilen sayiya kadar bak
            if(a[j]==rand_val){ //eger uretilen random sayi daha once varsa
                rand_val=rand()%shgl->shingle_count-1; //yenisini uret
                j=0; //en bastan kontrol etmek icin j yi sifirla
            } else{
                j++; //degilse bir sonraki indise
            }
        }
        a[i]=rand_val; //random sayiyi ekle
    }
    //hash matrisine degerler ataniyor
    for(i=0;i<shgl->shingle_count;i++){
        for(j=0;j<HASH_SIZE;j++){
            hash_mtr[i][j]=hash(a[j],i,shgl->shingle_count); //hash sayisi uret ve matrise yaz
        }
    }
    //imza matrisi olusturuluyor
    for(i=0;i<shgl->shingle_count;i++){//shingle gezer
        for(j=0;j<shgl->file_num;j++){//dosya gezer
            if(shgl->matrix[i][j]==1){ //secilen shingle hangi dosyada var j de tutuluyor
                for(k=0;k<HASH_SIZE;k++){ //ilgili shingle icin uretilen tum hashler
                    temp=hash_mtr[i][k]; //shingle icin uretilen h1,h2,h3... degerlerini tempe at
                    if(temp<signature[k][j]){ //imza matrisindeki dosya olan j dosyasinin k.hash degeriyle kiyasla kucukse
                        signature[k][j]=temp; //imza matrisindeki hash degerlerini ilgili dosya icin guncelle
                    }
                }
            }
        }
    } //imza matrisi tamamlandi boyutu [hash fonksiyonu sayisi][dosya sayisi]
    //minhasle bulunan benzerlik oranlarinin tutulacagi matris olusturuluyor
    shgl->minhash=(float**)malloc(shgl->file_num*sizeof(float*));
    for(i=0;i<shgl->file_num;i++){
        shgl->minhash[i]=(float*)calloc((size_t)shgl->file_num,sizeof(float));
    }
    //dosya ikilileri icin benzerlik oranlari signature similarity hesaplaniyor
    printf("\n\nK VALUE : %d THRESHOLD : %.2f\nSIGNATURE SIMILARITIES OF ALL DOCUMENT COMBINATIONS\n",shgl->k_val,shgl->threshold);
    for(i=0;i<shgl->file_num;i++){ //i. dosyanin
        for(j=i+1;j<shgl->file_num;j++){ //kendisi haric diger dosyalarla
            //printf("\nFile %d (%s) - File %d (%s) : ",i+1,files[i],j+1,files[j]);
            for(k=0;k<HASH_SIZE;k++){ //imza matrisi boyunca
                if(signature[k][i] == signature[k][j]) //benzer imza degerleri icin
                    intersect++; //toplami hesaplaniyor
            }
            shgl->minhash[i][j]=((float)intersect/(float)HASH_SIZE); //oran bulunuyor
            shgl->minhash[j][i]=shgl->minhash[i][j]; //bulunan oran matrisin diyagonaline de yaziliyor
            intersect=0;
        }
    }

    //Hesaplanan signature similarity oranlari tablo olarak ekrana yazdiriliyor
    printSignature(shgl,files);

    //free hash matrix
    for(i=0;i<shgl->shingle_count;i++){
        free(hash_mtr[i]);
    }
    free(hash_mtr);
    //free signature matrix
    for(i=0;i<HASH_SIZE;i++){
        free(signature[i]);
    }
    free(signature);

}

int main() {
    SHINGLE *shgl=(SHINGLE*)malloc(sizeof(SHINGLE)); //shingle yapisi olusturuluyor
    if(!shgl){
        printf("Shingle struct allocation error!!! Quitting...");
        exit(0);
    }
    int choice=1; //programin cikisi ve modul secimini tutan degisken
    int i; //cevrim degiskeni
    char filename[NAME_LEN]; //dosya adini tutan temp dizi
    printf("\nPlease enter file number : "); //dosya sayisi kullanicidan aliniyor
    scanf("%d",&shgl->file_num);
    char **files=(char**)malloc(sizeof(char*)*(shgl->file_num)); //dosya isimlerini tutacak dizi allocate ediliyor
    for(i=0;i<shgl->file_num;i++) { //dosya isimleri kullanicidan okunup kaydediliyor
        files[i]=(char*)malloc(NAME_LEN*sizeof(char));
        printf("\nPlease enter File name %d: ", i + 1);
        scanf("%s", filename);
        strcpy(files[i],filename);
    }
    printf("\nPlease enter your choice \n1-Continue with K-Value \n0-Exit\nChoice: ");
    scanf("%d",&choice); //secim kullanicidan okunuyor
    while(choice!=0){
        if(choice==1){  //devam etme secilirse
            printf("\nPlease enter K value of Shingles : ");
            scanf("%d",&shgl->k_val); //k degeri kullanicidan okunur
            printf("\nEnter threshold value for %d shingle similarity(0.0 between 1.0) : ",shgl->k_val);
            scanf("%f",&shgl->threshold); //threshold degeri kullanicidan okunur
            initialize(shgl);//shingle struct i icin ilk deger atamasi yapiliyor
            for(i=0;i<shgl->file_num;i++){
                FILE *fp=fopen(files[i],"r"); //sirayla dosyalar acilir
                if(!fp){
                    printf("File error!!! Quitting...");
                    exit(0);
                }
                createShingles(fp,i,shgl); //dosya dosya shinglelari structa olusturan fonksiyon
                fclose(fp);
            }
            //system("PAUSE");
            print_shingle(shgl,files); //dosyalarin shingle sayisi tablosunu ekrana yazdiran fonksiyon
            calculate_jaccard(shgl,files); //jaccard matrisi olusturuluyor ve sonuclarini yazdiriliyor
            calculate_minhash(shgl,files); //signature matrisi olusturuluyor ve sonuclarini yazdiriliyor
            //Free islemleri
            //free jaccard result matrix
            for(i=0;i<shgl->file_num;i++){
                free(shgl->jaccard[i]);
            }
            free(shgl->jaccard);
            //free minhashing results
            for(i=0;i<shgl->file_num;i++){
                free(shgl->minhash[i]);
            }
            free(shgl->minhash);
            //free shgl->matrix
            for(i=0;i<shgl->shingle_count;i++){
                free(shgl->matrix[i]);
            }
            free(shgl->matrix);
        }

        printf("\nPlease enter your choice \n1-Continue with K-Value \n0-Exit\nChoice: ");
        scanf("%d",&choice);
    }

    system("PAUSE");
    return 0;
}
