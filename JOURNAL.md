---
title: "Hell Countdown"
github: ""
description: ""
created_at: "2026-10-06"
total_time: "18h"
---

# October 17, 2026: 🛒 Picked out parts
<!-- fabricate:entry 579 -->

Hi, I'm Leo, and I’m building HellCountdown, a small pocket-sized device that counts down to the end of class. I already made a [web version](https://hellcountdown.netlify.app), and this is the physical one: an ESP32-S3 Mini with a 0.91" OLED, so I can keep it on my desk and watch the minutes drain away.

I spent ~1,5 hours on the 'birth' of the project; part selection. The brain is a Waveshare ESP32-S3 Mini, and the screen is a 128x32 SSD1306 OLED module. I chose a 400mAh Li-Po that already has a protection circuit built in, an MCP73831, and a PCF8563 real-time clock with a 32.768kHz crystal, which keeps time even when the device is off. Most parts came from DigiKey, with some parts from Grobotronics.

![](https://fabricate.hackclub-assets.com/05aa77970a17f81d5862439f99fc4f4a6dced11b88644105adf5d44089ab5d46/image-png/2026-10-17_19-46.png)

**Total time spent: 1h 30m**

# October 17, 2026 (entry 2): 🖌️ KiCad Schematic and PCB
<!-- fabricate:entry 600 -->

Today, I worked on the 'guts' of HCD; the schematic, and the PCB, with KiCad. After a lot of struggle with wiring, finding footprints, placing, tracing, and (kinda) passing DCR, I'm proud to see my little project embodied. I still need to design the case, however, which shouldn't be too hard. The parts cost approximately 60€.


![](https://fabricate.hackclub-assets.com/e181db1bbd95b27dd81f6ed1759eedbd4b682ff301540dbcff0479e79b8d8291/image-png/Screenshot_2026-10-17_21-29-45.png)

**Total time spent: 8h**

# October 25, 2026: 🔌 Added Buttons, and Rewired
<!-- fabricate:entry 1243 -->

I realized that I had forgotten to add a way for users to *interact* with HCD, so I added two small (6x3mm) buttons below the screen, to go through menus, settings, etc.
Additionally, after more thourougly reading the FAQ section, I realized automatic PCB routers were forbidden; as such, I removed ALL tracks, and wired the PCB by hand.
Finally, while updating and working on the BOM, I saw that DigiKey's shipping costs to Greece are 35€, not including customs - safe to say, I'm searching for a new electronics shop :')

![](https://fabricate.hackclub-assets.com/3d3510f6ceda9ecfc17dd4f25111b4d23da5129d3f953f766e634c161a337e20/image-png/sdoghsolfnd.png)

**Total time spent: 1h 30m**

# October 27, 2026: 🛒 Found new electronics provider
<!-- fabricate:entry 1324 -->

In light of the extremely high shipping costs from US warehouses, I found a provider to replace DigiKey with; [InterShop](https://www.inter-shop.gr), a Greek electronics e-shop, has most the components I need to make this project happen, with only negligent sacrifices as far as the design goes, and lowering the cost by 20% (to 69$/61€)

![](https://fabricate.hackclub-assets.com/53498013e6b222153cbc156664a6bda64865e3d117a96673d7210787dbd7316a/image-png/omgggggggggggggggg.png)

**Total time spent: 1h 30m**

# November 15, 2026: 🖨️ Made the case
<!-- fabricate:entry 3079 -->

I finally got to making, and finished, the case for the PCB and battery. I designed the case in two parts (top and bottom), so both sides have the same texture from the build plate, and to make it easier to assemble. What was a bit tricky, was getting the dimensions right, but I managed to place all the components realistically.

![](https://fabricate.hackclub-assets.com/caf5acbd68b7fff495d22d7285e22fa62d8ea0986ab20b790c2d80c8e3ee29c9/image-png/2026-11-15_16-32.png)
![](https://fabricate.hackclub-assets.com/2612ed47a147c5f42a4cd3d037efce60a91e05eaf27ca98c21366d3cc5ec3929/image-png/2026-11-15_16-32_1.png)
![](https://fabricate.hackclub-assets.com/f265cdce66376b9de441e47d6f855fc5fb47c77f1845a174e012a5633ff79009/image-png/2026-11-15_16-32_2.png)
![](https://fabricate.hackclub-assets.com/1531d5e4f3d2ea1a51684bb9c8a7d46c02c820cb7651e28647f55f2d89d3a95c/image-png/2026-11-15_17-11.png)
![](https://fabricate.hackclub-assets.com/d51f00410a5170245b035d7a9946d9a35db7485dca75fe20e163ad79a51cb101/image-png/2026-11-15_15-40.png)

**Total time spent: 2h**

# November 15, 2026 (entry 2): 🧑‍💻 Wrote the first firmware
<!-- fabricate:entry 3090 -->

I just finished the first firmware version of HellCountdown. While the logic it follows is tested with a similar Waveshare ESP32-S3 as used in this project and serial prints, the interaction between the ESP and the other components remains untested, until funding comes in, hence why it's a *first* version. Most of the time was spent trying to work through the bugs, as opposed to actually writing code; but I believe it's at a good first stage.

![](https://fabricate.hackclub-assets.com/7932f5bff3948ba3f8e8accc9ae3295e4f8ad8b25a789395253fdf2f57d2e19d/image-png/image.png)

**Total time spent: 2h 30m**

# November 15, 2026 (entry 3): 🐈‍⬛ Tidied up and published on GitHub
<!-- fabricate:entry 3121 -->

After reviewing the BOM, organizing into folders, shooting renders, and adding the necessary files, I have finally published the GitHub repository of the project.

![](https://fabricate.hackclub-assets.com/c92960d2a6b532d2dafb2abeeb5a8ad3d29d45bf73615e7d52d64fa624f9aa8a/image-png/image.png)

**Total time spent: 1h**
