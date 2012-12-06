%:- ['geos'].
:- use_module(library(geos)).

equal(X,Y) :-
        number(X),
        number(Y),
        !,
        X =:= Y.
equal((X1,Y1),(X2,Y2)) :-
        !,
        equal(X1,X2),
        equal(Y1,Y2).
equal([],[]).
equal([X|Xs],[Y|Ys]) :-
        !,
        equal(X,Y),
        equal(Xs,Ys).
equal(X,Y) :-
        functor(X,F,N),
        functor(Y,F,N),
        !,
        equal(N,X,Y).

equal(0,_,_).
equal(N,A,B) :-
        N > 0,
        arg(N,A,ARGA),
        arg(N,B,ARGB),
        equal(ARGA,ARGB),
        M is N - 1,
        equal(M,A,B).

test([]).
test([H|T]) :-
        functor(H,F,_),
        write('Testing '),
        write(F),
        write('... '),
        geos_test(H,H1),
        equal(H,H1), !,
        write('OK'), nl,
        test(T).
test([H|T]) :-
        write(H),
        write(' '),
        write('FAILED'), nl,
        test(T).


p(geometrycollection([])).
p(point(1,2)).
p(linestring([(1,2),(3,4)])).
p(polygon([[(1,2),(3,4),(4,5),(5,6),(1,2)]])).
p(polygon([[(1,2),(3,4),(4,5),(5,6),(1,2)],
           [(0,100),(1,101),(2,102),(0,100)]])).
p(multipoint([(0,10),(1,11),(2,12)])).
p(multilinestring([[(0,100),(1,101),(2,102)],
                   [(3,103),(4,104),(5,105)]])).
p(multipolygon([[[(0,10),(1,11),(2,12),(0,10)]],
                [[(13,3),(14,4),(15,5),(13,3)]]])).
p(geometrycollection([point(1,2),
                      linestring([(1,2),(3,4)]),
                      polygon([[(1,2),(3,4),(4,5),(5,6),(1,2)]]),
                      polygon([[(1,2),(3,4),(4,5),(5,6),(1,2)],
                               [(0,100),(1,101),(2,102),(0,100)]]),
                      multipoint([(0,10),(1,11),(2,12)]),
                      multilinestring([[(0,100),(1,101),(2,102)],
                                       [(3,103),(4,104),(5,105)]]),
                      multipolygon([[[(0,10),(1,11),(2,12),(0,10)]],
                                    [[(13,3),(14,4),(15,5),(13,3)]]])])).

initialize :-
        findall(X,p(X),L),
        assertz(p(geometrycollection(L))).

main :-
        write('=== Testing IMPORT/EXPORT ==='), nl,
        findall(X,p(X),L),
        test(L),
        write('=== DONE === '), nl,
        halt.
main :-
        write('=== INTERNAL ERROR ==='), nl.


memory :-
        findall(X,p(X),L),
        repeat,
        geos_test(geometrycollection(L),_),
        fail.

%% geos_intersection(polygon([[(0,0),(2,0),(0,2),(0,0)]]),
%%                   polygon([[(1,1),(-1,1),(1,-1),(1,1)]]),X).
%% X = polygon([[(1.0,0.0),(0.0,0.0),(0.0,1.0),(1.0,1.0),(1.0,0.0)]])
