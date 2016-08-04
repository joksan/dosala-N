.PHONY: generar
.PHONY: clean

generar: generador_z1.c
	gcc $< -trigraphs -o generador_z1
	./generador_z1

clean:
	rm generador_z1