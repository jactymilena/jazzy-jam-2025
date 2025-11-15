## maze.cpp

Script permettant de générer une map avec des mûrs. Actuellement cela retourne 
un tableau de centroid ainsi que la rotation de chaque mûr (H ou V).


```
g++ -std=c++20 maze.cpp -o maze

./maze             // Génération d'un fichier maxe.tex
pdflatex maze.tex  // Génération du PDF
zathura maze.pdf   // Visualisation du PDF
```

**TODO**:

* Faire fonctionner l<algorithme de génération pour des salles qui ne sont pas 
  carrées. 

* Augmenter le taux d'éllagage pour éviter les zones trop denses. 

* Ajouter des zones "locked" pour faire des zones innaccessibles (forme de T).
