# mpi_prim
V - lista wierzchołków
E - lista krawędzi
G = (V,E) - graf
p - liczba procesów
d_i - tablice odległosci


1) Inicjalizacja
	a) Dzielenie V na podmacierze gdzie każdy proces trzyma n/p wierzchołków, przydzielenie Vi i Ei do konkretnego procesu
	b) P0 tworzy drzewo rozpinające (MST) z dowolnego wierchołka i robi BCast do wszystkich pozostałych
	c) Każdy proces Pi tworzy tablice Di odległości jego części grafu
			Wybiera najkrótszą krawędź lub wpisuje inf.
2) Każdy proces Pi znajduje najkrótszą krawędź w swojej części grafu.
3) Każdy proces Pi wysyła najkrótszą krawędź do procesu P0.
4) P0 wybiera najkrótszą krawędź spośród przysłanych i dodaje do MST i robi BCast MST do reszty
5) Każdy proces Pi uaktualnia tablice Di według punktu 2.
6) Powtarzanie 2-5 dopóki nie MST = V
