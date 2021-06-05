descrição dos arquivos:

1) addresses.txt: arquivo que contem os endereços virtuais.
2) BACKING_STORE.bin: arquivo binário que contem 256 endereços com 256 bytes cada um.
3) vm.c: arquivo que contem o código do programa.
4) correct.txt: arquivo de saída que contem o endereço virtual, a posição da TLB, o endereço físico e o valor que está contido.

Para testar minha implementação, utilizei arquivos enviados pelo professor "addresses.txt" e "BACKING_STORE.bin", e 
o código criado por mim "vm.c". Executei eles através do makefile. O código foi implementado no VS Code e
testado no terminal do Linux. 
Para compilar, basta digitar o comando "make" e para rodar o programa "./vm" + o nome do arquivo que contem os endereços + os algoritmos de substituição da substituição de página e TLB, respectivamente. Será criado um arquivo de saída denominado "correct.txt", que irá conter todas as saídas do programa.