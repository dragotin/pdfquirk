#!/bin/bash


for lang in en de es
do
    lupdate-qt5 *cpp *h *ui -ts ../resources/pdfquirk_${lang}.ts
    lconvert-qt5 ../resources/pdfquirk_${lang}.ts -o ../resources/pdfquirk_${lang}.po
done


