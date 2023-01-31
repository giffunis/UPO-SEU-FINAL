# UPO-SEU-FINAL

Trabajo final de la asignatura.

---

## Descripción
---

El objetivo del proyecto es simular el comportamiento de un tren realizando un trayecto, involucrando para ello tres placas Arduino. Cada una de las placas ejercerá una función diferenciada y, mediante la comunicación entre las mismas, se generará un sistema completo e interconectado. El sistema se compondrá de un tren, que realiza un recorrido, pasando por un paso a nivel y llegando a una estación. Una de las placas será el tren, otra el paso a nivel y otra la estación de destino.
El tren dispondrá de sensor de temperatura e inclinación, que permitirá identificar si hay un descarrilamiento o sobrecalentamiento que obligue a pararse. Tendrá también como actuadores las luces, motor que simula el movimiento y un actuador sonoro que emite señales de alarma o aviso. El tren contará con un panel LCD que informe del tiempo restante de viaje y la velocidad.
El paso a nivel se encargará de cerrarse cuando el tren vaya a pasar, contando para ello con un servo que cierre el paso, aviso sonoro y luces led que hagan la función de semáforo.
Por último, la estación, que recibirá al tren. En la estación habrá un panel que informe del tiempo restante que le queda al tren para llegar y de la temperatura exterior, contando para ello con un sensor. Habrá también aviso lumínico y sonoro para avisar de la llegada del tren.
En cuanto a las comunicaciones, habrá varias. Por un lado, el tren informará de su velocidad y distancia recorrida, de tal modo que tanto la estación como el paso a nivel puedan utilizar esa información para sus paneles informativos o para activar sus actuadores. Del mismo modo, tanto la estación como el paso a nivel publicarán los datos del tiempo restante que le queda al tren para llegar o pasar con el objetivo de informar a sus usuarios por la página web o por aplicaciones que se suscriban a esta información. Si se produjera una incidencia en el tren que obligue a pararse, se desencadenará una acción para que tanto la estación como el paso a nivel estén informados. Las comunicaciones y datos se tratarán a través de la aplicación ThingSpeak y sus distintos componentes.