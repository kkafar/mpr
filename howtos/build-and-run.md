## Konfiguracja

Opis maszyn (hardware) i ich konfiguracji z których będzie korzystał działający program umieszcza się 
w pliku zwanym `machinefile`, który zwykle jest zwykłym plikiem tekstowym w formacie

```
<host>:<liczba procesorów>
<host>:<liczba procesorów>
```

Być może nie chodzi bezpośrednio o liczbę procesorów, a rdzeni na przykład. Nie wiem do końca jak to jest zwirtualizowane.
(Teraz wydaje mi się, że chodzi o "węzły")

Przykładowa konfiguracja:

```
vnode-01.dydaktyka.icsr.agh.edu.pl:4
vnode-02.dydaktyka.icsr.agh.edu.pl:4
vnode-03.dydaktyka.icsr.agh.edu.pl:4
vnode-04.dydaktyka.icsr.agh.edu.pl:4
vnode-05.dydaktyka.icsr.agh.edu.pl
vnode-06.dydaktyka.icsr.agh.edu.pl
vnode-07.dydaktyka.icsr.agh.edu.pl
vnode-08.dydaktyka.icsr.agh.edu.pl
vnode-09.dydaktyka.icsr.agh.edu.pl
vnode-10.dydaktyka.icsr.agh.edu.pl
vnode-11.dydaktyka.icsr.agh.edu.pl
vnode-12.dydaktyka.icsr.agh.edu.pl
```

## Kompilacja

W celu skompilowania programu korzystamy z kopmilatora `mpicc`, korzysta się z niego właściwie jak z `gcc`.

Przykładowa kompilacja:

```bash
mpicc -o <output_bin> <source_file> [source_files...]
```


## Wykonanie

Tak skompilowany program wykonuje się z użyciem `mpiexec`

Przykład:

```bash
mpiexec -machinefile <path_to_machine_file> -np <process_count> <executable_path>
```

Takie polecne uruchomi nam zestaw `<process_count>` jednakowych procesów. 

Jeżeli chcemy wykonać różne procesy, to wtedy trzeba użyć składni z dwukropkiem:

```bash
mpiexec -n <process_count_1> -host <host> <executable_1> : -n <process_count_2> -h <host> <executable_2>
```

Przydatne opcje do przetestowania to 

`-f`, `-configfile`, `-l`


**Ważne** jest to, że tak uruchomione procesy dzielą jeden wspólny komunikator `MPI_COMM_WORLD`

