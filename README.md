# UPO-SEU-FINAL

Trabajo final de la asignatura SEU - Sistemas Empotrados y Ubicuos

## Integrantes

- Gaspar Cauelo Cauelo.
- Adrian De Las Heras Cozar.
- José Saúl Giffuni Becerra.

## Descripción

El objetivo del proyecto es simular el comportamiento de un tren realizando un trayecto, involucrando para ello tres placas Arduino. Cada una de las placas ejercerá una función diferenciada y, mediante la comunicación entre las mismas, se generará un sistema completo e interconectado. El sistema se compondrá de un tren, que realiza un recorrido, pasando por un paso a nivel y llegando a una estación. Una de las placas será el tren, otra el paso a nivel y otra la estación de destino.

El tren tendrá también como actuadores el motor que simula el movimiento y un panel LCD que informe del tiempo restante de viaje y la velocidad.

El paso a nivel se encargará de cerrarse cuando el tren vaya a pasar, contando para ello con un servo que cierre el paso, aviso sonoro y luces led que hagan la función de semáforo. Además, faltando le indicará al tren de que debe bajar la velocidad durante un tramo determinado.

Por último, la estación, que recibirá al tren. En la estación habrá un panel que informe del tiempo restante que le queda al tren para llegar. Habrá también aviso lumínico y sonoro para avisar de la llegada del tren.

En cuanto a las comunicaciones, habrá varias. Por un lado, el tren informará de su velocidad y distancia recorrida, de tal modo que tanto la estación como el paso a nivel puedan utilizar esa información para sus paneles informativos o para activar sus actuadores. Tanto la estación como el paso a nivel podrán controlar la velocidad del tren publicando en el feed correspondiente. Las comunicaciones y datos se tratarán a través de la aplicación ThingSpeak y sus distintos componentes.

## Vídeo demostrando el funcionamiento

https://youtu.be/9iST7GG-7TE 