.PHONY: generar
.PHONY: clean

generar: generador.c
	gcc $< -trigraphs -o generador
	./generador

clean:
	rm generador