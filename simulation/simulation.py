#!/usr/bin/env python

import pygame
import random
import sys

pygame.init()

size = width, height = 1500, 1300
speed = [2, 2]
position = pygame.Rect(0, 0, 0, 0)
black = 0, 0, 0
gray = 0, 30, 75
white = 0xff, 0xff, 0xff
blue = 0, 20, 50

background = black
led_off = gray
led_on = white
sky = blue

screen = pygame.display.set_mode(size)

cloud = pygame.image.load("cloud.png")
cloud = pygame.transform.scale(cloud, (300, 150))

mode = 'snow'

led_radius = 2
led_panel = (240, 200)
led_spacing = (20, 25)

panels = [{
    'x': 100,  # to so koordinate prve ledice
    'y': 450,
    'height': 2,
    'stars': [
        (0, 0),
        (6, 3),
        (8, 6),
        (10, 9),
        (9, 12),

        (0, 15),
        (3, 14),
        (2, 8),
    ],
}, {
    'x': 550,
    'y': 175,
    'height': 2,
    'stars': [
        (7, 6),
        (7, 8),
        (2, 6),
        (3, 8),
        (11, 9),

        (3, 15),
        (4, 13),
        (2, 2),
        (5, 0),
        (9, 1),
    ],
}, {
    'x': 450,
    'y': 720,
    'height': 1,
    'stars': [
        (1, 2),

        (11, 1),
        (8, 7),
    ],
}, {
    'x': 1200,
    'y': 275,
    'height': 3,
    'stars': [
        (4, 17),
        (8, 16),
        (9, 13),
        (11, 9),

        (3, 0),
        (1, 3),
        (5, 3),
        (3, 6),
        (11, 20),
        (6, 23),
    ],
}, {
    'x': 880,
    'y': 540,
    'height': 2,
    'stars': [
        (0, 0),

        # hp
        (9, 2),
        (8, 4),
        (7, 6),
        (6, 8),
        (8, 7),
        (10, 6),
        (9, 8),
        (8, 10),
        (7, 12),

        (3, 14),
        (2, 8),
    ],
}]

additional_stars = [
    (383, 833),
    (838, 464),
    (1468, 581),

    # kefej
    (1085, 30),
    (1253, 90),
    (979, 167),
    (1128, 247),
    (939, 381),

    # drugo
    (1161, 75),
    (1131, 77),
    (1121, 129),
    (1111, 152),
    (991, 219),
    (1008, 264),
    (968, 326),
    (944, 45),
    (821, 48),
    (850, 266),
    (1378, 42),
    (1409, 91),
    (1468, 42),
    (450, 50),
    (488, 94),
    (419, 145),
    (329, 146),
    (318, 109),
    (232, 106),
    (138, 93),
    (56, 83),
    (96, 190),
    (234, 191),
    (252, 261),
    (388, 261),
    (470, 395),
    (429, 543),
    (44, 738),
    (734, 732),
    (811, 667),
    (840, 817),
    (786, 807),
    (1169, 626),
    (1142, 744),
    (1166, 846),
    (624, 645),
    (64, 584),
    (60, 322),
    (266, 866),
    (1469, 748),
    (1455, 320),
    (1481, 346),
    (1102, 348),
    (1117, 385),
    (198, 36),
    (142, 253),
    (405, 453),
    (506, 486),
    (1036, 106),
]


for panel in panels:
    panel['animation'] = []
    panel['next'] = 0
    panel['ledsw'] = led_panel[0] / led_spacing[0]
    panel['ledsh'] = led_panel[1] * panel['height'] / led_spacing[1]
    panel['leds'] = [
        [panel['x'] + led_spacing[0]*x, panel['y'] + led_spacing[1]*y] for y in range(panel['ledsh']) for x in range(panel['ledsw'])
    ]

print(sum(len(panel['leds']) for panel in panels))

clock = pygame.time.Clock()
max_animation = 0


def anim_map(an):
    x = an[0] + an[2]
    y = an[1] + an[3]
    return x, y, an[2], an[3]


max_len = 0
while 1:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            sys.exit()
        elif event.type == pygame.MOUSEBUTTONDOWN:
            print(pygame.mouse.get_pos())
        elif event.type == pygame.KEYDOWN and event.key == pygame.K_a:
            if mode == 'snow':
                mode = 'stars'
            else:
                mode = 'snow'

    screen.fill(background)
    pygame.draw.rect(screen, sky, pygame.Rect(0, 0, 1500, 900))

    for panel in panels:
        if len(panel['animation']) > max_len:
            max_len = len(panel['animation'])
            print(max_len)
        panel['animation'] = filter(
            lambda an: round(an[0]) >= 0 and round(an[0]) < panel['ledsw'] and int(an[1]) < panel['ledsh'],
            map(anim_map, panel['animation']))

        panel['next'] -= 1
        if panel['next'] <= 0:
            vabs = max(0.01, random.normalvariate(0.07, 0.01))
            y = led_panel[1] * panel['height']
            x = led_panel[0] * random.normalvariate(0, 0.5)
            c = (x**2+y**2)**0.5
            vx = x*vabs/c
            vy = y*vabs/c

            panel['animation'].append([
                random.randrange(panel['ledsw']),  # x
                0,  # y
                vx,
                vy,
            ])
            panel['next'] = random.gammavariate(3, 5)
            if len(panel['animation']) > max_animation:
                max_animation = len(panel['animation'])
                print(max_animation)

        for led in panel['leds']:
            pygame.draw.circle(screen, led_off, led, led_radius)

        cloudrect = cloud.get_rect()
        cloudrect = cloudrect.move([panel['x'] - 30, panel['y'] - 150])
        screen.blit(cloud, cloudrect)
        if mode == 'snow':

            for an in panel['animation']:
                index = int(round(an[0]) + panel['ledsw'] * int(an[1]))
                pygame.draw.circle(screen, led_on, panel['leds'][index], led_radius)
        else:
            for panel in panels:
                for star in panel['stars']:
                    index = star[0] + panel['ledsw'] * star[1]
                    pygame.draw.circle(screen, led_on, panel['leds'][index], led_radius)
            for star in additional_stars:
                pygame.draw.circle(screen, led_on, star, led_radius)

    pygame.display.flip()

    clock.tick(60)
