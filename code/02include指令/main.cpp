#include<iostream>

int main(){
    int arr[] = {
#include"array.txt"
    };
    for(int i = 0; i < sizeof(arr)/sizeof(int); ++i)
            std::cout<< arr[i] <<' ';
    std::cout<<'\n';
}