# ReproductorWAVconCamara

Maria Fernanda Triviño, Andrés Felipe Galeano, Cristian Camilo Camargo

En este documento se describirá el funcionamiento y la arquitectura de nuestro proyecto para lectrónica digital II, en la Universidad Nacional de Colombia. El fin del proyecto es implementar un SoC con un procesador LM32 y varios periféricos integrados. Los periféricos siguen una lógica de maestro-esclavos y se unen al procesador LM32 mediante un bus wishbone.
A continuación se muestra el diagrama de funcionamiento del SoC, donde se usa un procesador LM32, se interconectan los periféricos por medio de un wishbone y se realizan los protocolos de comunicación SPI, UART y GPIO.

https://raw.githubusercontent.com/afgaleanog/ReproductorWAVconCamara/master/reproductorwav.png

# Especificaciones:

El proyecto tiene las siguientes especificaciones:

    Lector de micro SD de hasta 8 GB. Realiza la inicialización y lectura de datos de una memoria SD ver 2.
    Pantalla LCD de 176 x 220 pixeles. Muestra el juego de Snake.
    5 botones pulsadores. B1: movimiento a la derecha. B2: movimiento a la izquierda. B3: movimiento arriba. B4: movimiento abajo. B5: Reinicia juego.
    Procesador LM32 con frecuencia de 100 MHz. Con bus de datos de 32 bits, realizar el procesamiento del juego y controla los distintos periféricos de la consola.
    Memoria RAM de 17 Kbits ó 2 KB. Almacena la lógica del juego.
    Alimentación a 5V.

Prototipado y pruebas
LCD-SPI

Este periférico controla la pantalla por la cual se va a mostrar la imagen del juego. El dispositivo a usar es la LCD ILI9225 que recibe información con una comunicación SPI.La pantalla es controlada por medio del módulo SPI que proporciona Litex y posee una resolución de 176x220 pixeles RGB.

Se observó en el datasheet que para el modo normal de escritura se necesitaba un ancho mínimo de pulso de escritura en flanco de subida de 50ns y un tiempo mínimo del ciclo del bus de escritura de 100 ns; por lo tanto se decidió un periodo del spi-clock de 160 ns que corresponden a una frecuencia de 6.25MHz (para asegurar que los datos se envíen completos). Estos arreglos se ven reflejados en la variable $config$ en la función de configuración en el archivo main.c

De igual manera se estableció la longitud de escritura alterando la variable $xfer$. Se estableció un envío serial de 16 bits.

Si bien el módulo se realiza por hardware, la inicialización de la LCD se desarrolló por software debido a las ventajas que presenta el software en el momento de implementar cambios del programa. Por ejemplo, si se desea cambiar el modo de escritura de la pantalla que se encuentra en la parte de inicialización de la misma, no se requiere sintetizar de nuevo (tarea que requiere una cantidad considerable de tiempo) sino simplemente compilar y actualizar el main.c en la fpga. Esta versatilidad también se ve reflejada cuando nos disponemos al crear tarjetas hija. Si se realizara la inicialización por hardware la LCD quedaría restringida a trabajar de una sola manera. Cabe aclarar que la configuración y la inicialización solo se realizan una vez.

Para habilitar la pantalla a la recepción de datos es necesario que el $Cs$ se active en low durante la transmisión, el $rst$ de esta señal se mantiene en 1 lógico (polarización inversa) y el pin $rs$ de la pantalla es ajustado según su tarea, es decir, 1 para el envío de la dirección de memoria que se desea modificar y 0 para el dato a escribir. Como el pin $rs$ es un pin físico en la lcd, se instanció como un módulo del Soc para poder acceder a él desde software por medio de los registros de control.

Finalmente, después de configurar e inicializar, la pantalla queda lista para la recepción pixel a pixel de los colores RGB. En esta etapa no se han creado los tiles a enviar a la LCD por lo que desde nuestro archivo main.c se crearon funciones que dibujan distintas figuras, haciendo que nuestro procesador envíe pixel a pixel para pintar la pantalla. El reto es lograr que el procesador lm32 administre una fifo que contiene los tiles de la pantalla, estos tiles se leeran de la SD y se almacenarán en una memoria de nuestro SoC. Desde allí serán enviadas por medio de una fifo a nuestra LCD . De esta manera, se realiza la impresión de la imagen por hardware para no ocupar el software en una tarea constante como la de mantener la pantalla pintada.
SD card-spi

El juego se va a leer desde una tarjeta de memoria SD Card con ayuda del protocolo de comunicacion SPI, de la misma manera en la que se inicializó la pantalla LCD. Con este periférico se envían los datos de inicialización al lector SD, el cual será un modulo micro sd por medio del cual se realizaran las conexiones del mosi, miso, sclk y cs. Para conectar el modulo con nuestro procesador se utiliza el modulo SPI que proporciona Litex, el cual se tratará más adelante. Por medio de este hardware se realiza la inicialización desde software, con lo cual se configura una frecuencia para la inicialización de 400kHz mediante el divisor de escritura y lectura del registro $config$ y con el registro $xfer$ se determinó la cantidad de bits de escritura como 8, ya que el formato de comando consta de 48 bits y se quiere enviar 6 datos de 8 bits. Para realizar la inicialización de la sd se debe primero esperar 74 ciclos de reloj para garantizar que se energice la memoria, lo siguiente en configurar es entrar en el estado "IDLE", para ello se manda el comando 0 (CMD0) con 0 de argumento y 0x95 como CRC7. La memoria debe responder con un uno. El segundo paso es enviar las condiciones de operación, las cuales se pueden estrucutrar dependiendo del la memoria, siendo CMD1 si es MMC o ACMD41 si es SD card, por lo que en nuestro caso enviamos el ACMD41. Para enviar un ACMD se debe enviar primero el CMD55 y depués el CMD41, con lo que se espera una respuesta de 0 por parte de la sd card. Una vez establecidas las condiciones de operación la inicialización está terminada, es decir, se puede empezar a escribir en la memoria; para ello se tiene predeterminado una longitud de escritura de 512 bytes pero por medio del comando 16 (CMD16) se puede definir la longitud en el argumento. Para enviar los datos que se desean escribir, se envía el comando 24 (CMD24), donde en su argumento se pone la dirección donde se quiere escribir, y seguido de este comando se manda el bloque de datos; para terminar el ciclo de lectura se envía el comando 12 (CMD12) que informa el fin de la transmisión. Para leer de la memoria se usa el CMD17 y en el argumento se pone la dirección a la cual se quiere acceder. La memoria enviará el bloque de lectura establecido previamente con el CMD16 y un CRC16 que determinará el fin de la transmisión.
GPI

Para establecer la dinámica con el usuario, se disponen 5 botones establecidos de la siguiente manera: 4 para las direcciones (arriba, abajo, derecha, izquierda) y un boton que restaura el juego a su estado inicial.

En primer lugar se decidió testear el funcionamiento de 16 switches para relacionarlos con el color de la pantalla LCD y así corroborar su correcto uso. Haciendo uso del módulo GPIO de Litex se instanciaron 16 switches con el fin de probarlos mediante la terminal. Este periférico no necesita configuración predeterminada ni inicialización a diferencia de los dos anteriores. Su funcionamiento consiste en simplemente usar la función generada en el csr.h para leer el estado de los botones. Utilizando este mismo periférico, pero en modo escritura, se instanció el pin rs de la LCD.
GPO

Para establecer pines sin cambios constantes como es el caso del RS y RST de la LCD se decide implementarlos por medio de modulos GPO los cuales presentan valos no cambiantes en el código. Para establecer un CS variable según la necesidad del protocolo se decide establecerlo también como un GPO ya que si lo usabamos con la salida regular del módulo SPI se cambiaba cada que se enviaba un arreglo de datos, y se necesitaba que se cambiara cuando la trama de 48 bits fuera enviada, por eso se decidió implementar de esta forma.
Documentación
Funcionamiento

El proyecto cuenta con 3 protocolos de comunicación como SPI, GPI y UART con los que se enlazan los periféricos físicos externos al procesador como la pantalla LCD, memoria SD y los pulsadores con el control LM32. Por otro lado, se consideran los siguientes módulos como periféricos:

    LCD-SPI
    SDcard-SPI
    GPI
    Timer
    UART

El mapa de memoria de nuestro proyecto es el siguiente:

mapamem Para abordar en estos periféricos de comunicación se analizará primero el procesador LatticeMico32.
LatticeMico32

lm

En la figura anterior se puede observar el funcionamiento del procesador, el cual se divide en 6 etapas de funcionamiento.

    La primera es direcciones, donde se calcula y envía al contador de programa la dirección de la instrucción a ejecutar.
    Fech, es donde la instrucción es leida desde la memoria y es almacenada en el registro de instrucción.
    La decodifica y los operandos también son entregados al resto del proceso mientras que a la par el PC relativo a las ramas es predicho.
    Se ejecuta la operación decodificada y para operaciones lógicas simples o sumas la ejcución termina y se entrega.
    Para instrucciones más complicadas, se almacena el resultado y una segunda ejecución es realizada para obtener el resutado final.
    Finalmente los resultados son escritos en el archivo de registros.

Para el control de interrupciones se designan 3 registros por parte del procesador, esos son el habilitador de interrupciones (IE), la máscara de interrupciones (IM) y la interrupción pendiente (IP). Una vez activada una interrupción, el procesador guarda todos sus registros en la pila, modifica el registro IP y espera hasta que se solucione la interrupción para restaurar el valor de los registros almacenados y retomar el programa inicial. La conexión entre los periféricos y el procesador se hace por medio de un bus wishbone. Sin embargo las interrupciones realizadas por los periféricos son directas con el procesador para que pueda encargarse de ellas inmediatamente.
Bus wishbone

Como se mencionó antes, los periféricos y el procesador se comunican por medio de un bus wishbone. A continuación podemos ver las entradas y salidas que necesita dicho bus para su adecuado funcionamiento.

wishbone
SPI

El periferico SPI implementado es el nos proporcionó el profesor y se puede observar en la figura siguiente el diagrama de bloques funcional donde se especifican los registros y los subbloques que lo componen. Nuestro periferico SPI se instancia dos veces, uno para la LCD y otro para la comunicación con la SD donde la única diferencia entre los dos es la configuración de frecuencias y cantidad de bits por tiempo de escritura. spiBloq
LCD

Al instanciar la LCD como un módulo SPIMaster se obtuvo el siguiente mapa de memoria:

lcd
SD

Nuestro mapa de memoria quedó configurado de la siguiente manera:

sd
GPI

Para el protocolo GPI se manejó solo un registro donde se almacenaba el estado de los pulsadores y por medio de las interrupciones se gestiona la ejecución de las acciones. De aquí se encontró el siguiente mapa de memoria:

gpio
Timer

Nuestro periferico timer se encuentra ubicado en las siguientes posiciones de memoria: timer
UART

Para conectarnos enviar y recibir información desde la uart se usan los siguientes registros: uart
