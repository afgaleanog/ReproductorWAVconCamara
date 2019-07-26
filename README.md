# ReproductorWAVconCamara

Maria Fernanda Triviño, Andrés Felipe Galeano, Cristian Camilo Camargo

En este documento se describirá el funcionamiento y la arquitectura de nuestro proyecto para lectrónica digital II, en la Universidad Nacional de Colombia. El fin del proyecto es implementar un SoC con un procesador LM32 y varios periféricos integrados. Los periféricos siguen una lógica de maestro-esclavos y se unen al procesador LM32 mediante un bus wishbone.
A continuación se muestra el diagrama de funcionamiento del SoC, donde se usa un procesador LM32, se interconectan los periféricos por medio de un wishbone y se realizan los protocolos de comunicación SPI, UART y GPIO.<br />

![](https://raw.githubusercontent.com/afgaleanog/ReproductorWAVconCamara/master/reproductorwav.png)

# Especificaciones:

El proyecto tiene las siguientes especificaciones:



# Perifericos

• SPI: Para el control de la pantalla y la ranura SD.
• I2C: Para el control del acelerómetro.
• GPIO: Para el control de los botones.
• UART: Para la programación de la tarjeta.
• TIMER: Para el control del reloj de la tarjeta. 


# Documentación

Funcionamiento

El modulo tiene 3 protocolos de comunicación: SPI, GPI y UART con los que se enlazan los periféricos externos al procesador: pantalla LCD, memoria SD y los pulsadores con el control LM32. Por otro lado, se consideran los siguientes módulos como periféricos:

    LCD-SPI
    SDcard-SPI
    GPI
    Timer
    UART

El mapa de memoria del proyecto se muestra a continuación:

![](https://raw.githubusercontent.com/afgaleanog/ReproductorWAVconCamara/master/mapadememoria.png)


MEMORIA FIFO
El formato VGA tiene (640)píxeles verticales
Un pixel en raw ocupa 3 Byte->640*3=0X780 bytes

Para los periféricos de comunicación se estudiará el procesador LatticeMico32.

![](https://raw.githubusercontent.com/afgaleanog/ReproductorWAVconCamara/master/lm32.png)

En la figura anterior se puede observar el funcionamiento del procesador, el cual se divide en 6 etapas de funcionamiento.

    La primera es direcciones, donde se calcula y envía al contador de programa la dirección de la instrucción a ejecutar.
    Fech, es donde la instrucción es leida desde la memoria y es almacenada en el registro de instrucción.
    La decodifica y los operandos también son entregados al resto del proceso mientras que a la par el PC relativo a las ramas es predicho.
    Se ejecuta la operación decodificada y para operaciones lógicas simples o sumas la ejcución termina y se entrega.
    Para instrucciones más complicadas, se almacena el resultado y una segunda ejecución es realizada para obtener el resutado final.
    Finalmente los resultados son escritos en el archivo de registros.

Para el control de interrupciones se designan 3 registros por parte del procesador, esos son el habilitador de interrupciones (IE), la máscara de interrupciones (IM) y la interrupción pendiente (IP). Una vez activada una interrupción, el procesador guarda todos sus registros en la pila, modifica el registro IP y espera hasta que se solucione la interrupción para restaurar el valor de los registros almacenados y retomar el programa inicial. La conexión entre los periféricos y el procesador se hace por medio de un bus wishbone. Sin embargo las interrupciones realizadas por los periféricos son directas con el procesador para que pueda encargarse de ellas inmediatamente.


# Bus wishbone

![](https://raw.githubusercontent.com/afgaleanog/ReproductorWAVconCamara/master/wishbone.png)

SPI

El periferico SPI implementado es el proporcionado por el profesor y se puede observar en la figura siguiente el diagrama de bloques funcional donde se especifican los registros y los subbloques que lo componen. Nuestro periferico SPI se instancia dos veces, uno para la LCD y otro para la comunicación con la SD donde la única diferencia entre los dos es la configuración de frecuencias y cantidad de bits por tiempo de escritura.






