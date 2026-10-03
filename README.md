# ESP32 Learning Project

This is a small educational project for the [Embedded Development course](https://beetroot.academy/courses/online/kurs-embedded-development)



## Scheme
```
ESP32-S3

                    +----------------------+
3.3V ---------------|                      |
                    |      ESP32-S3        |
GPIO4 --------------|                      |
GPIO5 --------------|                      |
GPIO8 --------------|                      |
GPIO9---------------|                      |
                    |                      |
                    |                      |
                    |                      |
GND ----------------|                      |
                    +----------------------+


Button

                 +3.3V
                   │
                [BUTTON]
                   │
GPIO4 ─────────────┼───────────────
                   │
                [10kΩ]
                   │
                  GND


Beeper

+--------+
| Beeper |- + --> 3.3V
|        |- S --> GPIO 5
|        |- - --> GND
+--------+



LCD SSD1306-Revision 1.1

+--------+
|  LCD   |-- SCK ----> GPIO 9
|        |-- SDA ----> GPIO 8
|        |-- VDD ----> 3.3V
|        |-- GND ----> GND
+--------+



```

## Thanks
- ЗСУ
- Beetroot Academy
- Copilot with GPT-5.4 mini
- [image2cpp tool](https://javl.github.io/image2cpp/)
- [Ivanka](https://www.youtube.com/@paper-youtube-ua) for the design of the device case


## Result

![Result scheme](./result_01.gif)

If GIF preview is not displayed in your viewer, open it directly: [result_01.gif](./result_01.gif)


![Result device](./result_02.mp4)

If GIF preview is not displayed in your viewer, open it directly: [result_02.mp4](./result_02.mp4)