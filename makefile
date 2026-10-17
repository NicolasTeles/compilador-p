leitor: leitor.c leitor.h lexer.h afd_tabela.h
	gcc leitor.h lexer.h afd_tabela.h leitor.c -o leitor

clean:
	rm -rf leitor leitor.o