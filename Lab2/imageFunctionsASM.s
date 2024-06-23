; imageFunctionsASM.s
; Desenvolvido para a placa EK-TM4C1294XL
; Histograma para imagens de até 64K pixels
; Pedro Henrique Grossi da Silva
; 23/06/2024

; -------------------------------------------------------------------------------------------------------------------------
; Funções
; -------------------------------------------------------------------------------------------------------------------------

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  EightBitHistogram_ASM

IMAGE_SIZE_LIMIT EQU 0xFFFF

;------------EightBitHistogram_ASM------------
; Gera histograma para imagens até 64K. 
; Caso a imagem seja maior que 64K retorna 0, 
; Caso contrario retorna o tamanho da imagem.
; Usa o ponteiro p_histogram como retorno do histograma
; Entrada: R0, R1, R2, R3
; Saída: R0
; Modifica: R0, R1, R2, R3
EightBitHistogram_ASM
    MULS  R1, R0, R1
	MOV   R0, #IMAGE_SIZE_LIMIT
	CMP   R1, R0                 ; Verifica se o tamanho da imagem pode ser processado
	MOVEQ R0, #0
	BHS   Return                 ; Se for maior ou igual retorna a função
    MOV   R0, #0                 ; i = 0
	PUSH  {R4, R5, R6}           ; Salva R4, R5 e R6
	MOV   R4, #0                 ; Valor para limpar a memoria
ClearHistogram
    STR   R4, [R3, R0, LSL #1]   ; Desloca o vetor e limpa a memoria
	ADD   R0, #1                 ; i++
	CMP   R0, #255               ; Verifica se chegou ao ultimo elemento
	BNE   ClearHistogram
	MOV   R0, #0                 ; i = 0;
CalculateHistogram
    LDRB  R5, [R2, R0]           ; Busca o valor de R2 deslocado
	LDRH  R6, [R3, R5, LSL #1]   ; Busca o valor de R3 deslocado
    ADD   R6, #1
	STRH  R6, [R3, R5, LSL #1]   ; Armazena o valor incrementado
	ADD   R0, #1                 ; i++
	CMP   R0, R1                 ; Verifica se chegou ao final da imagem
	BNE   CalculateHistogram
	POP   {R4, R5, R6}           ; Retorna os valores de R4, R5 e R6
Return
	BX LR

; -------------------------------------------------------------------------------------------------------------------------
; Fim do Arquivo
; -------------------------------------------------------------------------------------------------------------------------
    ALIGN                        		;Garante que o fim da seção está alinhada 
    END                          		;Fim do arquivo