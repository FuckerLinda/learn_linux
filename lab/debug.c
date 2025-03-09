#include<stdio.h>
void main(){
	int arr[10],i=0;
	for(i=0;i<10;i++){
		arr[i]=i;
		if (DEBUG){
			printf("arr[%d]=%d\n",i,arr[i]);
	}
	}
}
