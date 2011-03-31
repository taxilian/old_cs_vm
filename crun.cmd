@echo off
Debug\lexer.exe %1
Debug\virtualmachine.exe %1.out 2>tmp
