#!/usr/bin/env python3
"""Generate step 1–5, 7 schematics — parameterstyrd."""
import schemdraw
import schemdraw.elements as elm
import schemdraw.logic as logic
import matplotlib
matplotlib.use('Agg')

OUT = '/var/www/6502.smutje.se/schematics'

class W65C02S(elm.Ic):
    def __init__(self, *args, **kwargs):
        pins = [
            elm.IcPin(name='VPB',   side='l', pin='1',  slot='20/20'),
            elm.IcPin(name='RDY',   side='l', pin='2',  slot='19/20'),
            elm.IcPin(name='PHI1O', side='l', pin='3',  slot='18/20'),
            elm.IcPin(name='IRQB',  side='l', pin='4',  slot='17/20'),
            elm.IcPin(name='MLB',   side='l', pin='5',  slot='16/20'),
            elm.IcPin(name='NMIB',  side='l', pin='6',  slot='15/20'),
            elm.IcPin(name='SYNC',  side='l', pin='7',  slot='14/20'),
            elm.IcPin(name='VDD',   side='l', pin='8',  slot='13/20'),
            elm.IcPin(name='A0',    side='l', pin='9',  slot='12/20'),
            elm.IcPin(name='A1',    side='l', pin='10', slot='11/20'),
            elm.IcPin(name='A2',    side='l', pin='11', slot='10/20'),
            elm.IcPin(name='A3',    side='l', pin='12', slot='9/20'),
            elm.IcPin(name='A4',    side='l', pin='13', slot='8/20'),
            elm.IcPin(name='A5',    side='l', pin='14', slot='7/20'),
            elm.IcPin(name='A6',    side='l', pin='15', slot='6/20'),
            elm.IcPin(name='A7',    side='l', pin='16', slot='5/20'),
            elm.IcPin(name='A8',    side='l', pin='17', slot='4/20'),
            elm.IcPin(name='A9',    side='l', pin='18', slot='3/20'),
            elm.IcPin(name='A10',   side='l', pin='19', slot='2/20'),
            elm.IcPin(name='A11',   side='l', pin='20', slot='1/20'),
            elm.IcPin(name='VSS',   side='r', pin='21', slot='1/20'),
            elm.IcPin(name='A12',   side='r', pin='22', slot='2/20'),
            elm.IcPin(name='A13',   side='r', pin='23', slot='3/20'),
            elm.IcPin(name='A14',   side='r', pin='24', slot='4/20'),
            elm.IcPin(name='A15',   side='r', pin='25', slot='5/20'),
            elm.IcPin(name='D7',    side='r', pin='26', slot='6/20'),
            elm.IcPin(name='D6',    side='r', pin='27', slot='7/20'),
            elm.IcPin(name='D5',    side='r', pin='28', slot='8/20'),
            elm.IcPin(name='D4',    side='r', pin='29', slot='9/20'),
            elm.IcPin(name='D3',    side='r', pin='30', slot='10/20'),
            elm.IcPin(name='D2',    side='r', pin='31', slot='11/20'),
            elm.IcPin(name='D1',    side='r', pin='32', slot='12/20'),
            elm.IcPin(name='D0',    side='r', pin='33', slot='13/20'),
            elm.IcPin(name='RWB',   side='r', pin='34', slot='14/20'),
            elm.IcPin(name='NC',    side='r', pin='35', slot='15/20'),
            elm.IcPin(name='BE',    side='r', pin='36', slot='16/20'),
            elm.IcPin(name='PHI2',  side='r', pin='37', slot='17/20'),
            elm.IcPin(name='SOB',   side='r', pin='38', slot='18/20'),
            elm.IcPin(name='PHI2O', side='r', pin='39', slot='19/20'),
            elm.IcPin(name='RESB',  side='r', pin='40', slot='20/20'),
        ]
        super().__init__(pins=pins, w=5, plblofst=.05, botlabel='W65C02S CPU', **kwargs)

class ArduinoMega2560(elm.Ic):
    def __init__(self, *args, **kwargs):
        pins = [
            # Vänster sida
            elm.IcPin(name='D4_RESET',pin='4',   side='l', slot='40/40'),
            elm.IcPin(name='D2_PHI2', pin='2',   side='l', slot='37/40'),
            elm.IcPin(name='D3_RW',   pin='3',   side='l', slot='34/40'),
            elm.IcPin(name='D22_D0',  pin='22',  side='l', slot='32/40'),
            elm.IcPin(name='D23_D1',  pin='23',  side='l', slot='31/40'),
            elm.IcPin(name='D24_D2',  pin='24',  side='l', slot='30/40'),
            elm.IcPin(name='D25_D3',  pin='25',  side='l', slot='29/40'),
            elm.IcPin(name='D26_D4',  pin='26',  side='l', slot='28/40'),
            elm.IcPin(name='D27_D5',  pin='27',  side='l', slot='27/40'),
            elm.IcPin(name='D28_D6',  pin='28',  side='l', slot='26/40'),
            elm.IcPin(name='D29_D7',  pin='29',  side='l', slot='25/40'),
            *[elm.IcPin(name=f'A{8+i}_A{8+i}', pin=f'A{8+i}', side='l', slot=f'{23-i}/40') for i in range(8)],
            *[elm.IcPin(name=f'A{i}_A{i}', pin=f'A{i}', side='l', slot=f'{15-i}/40') for i in range(8)],
            elm.IcPin(name='D13_SYNC',pin='13',  side='l', slot='6/40'),
            # Höger sida
            elm.IcPin(name='D11_BTN1',pin='11',  side='r', slot='36/40'),
            elm.IcPin(name='D12_BTN2',pin='12',  side='r', slot='32/40'),
            elm.IcPin(name='SDA',     pin='SDA', side='r', slot='24/40'),
            elm.IcPin(name='SCL',     pin='SCL', side='r', slot='20/40'),
            elm.IcPin(name='V5',      pin='5V',  side='r', slot='8/40'),
            elm.IcPin(name='GND_pin', pin='GND', side='r', slot='4/40'),
        ]
        super().__init__(pins=pins, w=6, plblofst=.05, botlabel='Arduino Mega 2560', **kwargs)

class SRAM62256(elm.Ic):
    def __init__(self, *args, **kwargs):
        pins = [
            elm.IcPin(name='A14',  side='l', pin='1',  slot='14/14'),
            elm.IcPin(name='A12',  side='l', pin='2',  slot='13/14'),
            elm.IcPin(name='A7',   side='l', pin='3',  slot='12/14'),
            elm.IcPin(name='A6',   side='l', pin='4',  slot='11/14'),
            elm.IcPin(name='A5',   side='l', pin='5',  slot='10/14'),
            elm.IcPin(name='A4',   side='l', pin='6',  slot='9/14'),
            elm.IcPin(name='A3',   side='l', pin='7',  slot='8/14'),
            elm.IcPin(name='A2',   side='l', pin='8',  slot='7/14'),
            elm.IcPin(name='A1',   side='l', pin='9',  slot='6/14'),
            elm.IcPin(name='A0',   side='l', pin='10', slot='5/14'),
            elm.IcPin(name='D0',   side='l', pin='11', slot='4/14'),
            elm.IcPin(name='D1',   side='l', pin='12', slot='3/14'),
            elm.IcPin(name='D2',   side='l', pin='13', slot='2/14'),
            elm.IcPin(name='GND',  side='l', pin='14', slot='1/14'),
            elm.IcPin(name='VCC',  side='r', pin='28', slot='14/14'),
            elm.IcPin(name='WEB',  side='r', pin='27', slot='13/14'),
            elm.IcPin(name='A13',  side='r', pin='26', slot='12/14'),
            elm.IcPin(name='A8',   side='r', pin='25', slot='11/14'),
            elm.IcPin(name='A9',   side='r', pin='24', slot='10/14'),
            elm.IcPin(name='A11',  side='r', pin='23', slot='9/14'),
            elm.IcPin(name='OEB',  side='r', pin='22', slot='8/14'),
            elm.IcPin(name='A10',  side='r', pin='21', slot='7/14'),
            elm.IcPin(name='CEB',  side='r', pin='20', slot='6/14'),
            elm.IcPin(name='D7',   side='r', pin='19', slot='5/14'),
            elm.IcPin(name='D6',   side='r', pin='18', slot='4/14'),
            elm.IcPin(name='D5',   side='r', pin='17', slot='3/14'),
            elm.IcPin(name='D4',   side='r', pin='16', slot='2/14'),
            elm.IcPin(name='D3',   side='r', pin='15', slot='1/14'),
        ]
        super().__init__(pins=pins, w=5, plblofst=.05, botlabel='62256 SRAM (32KB)', **kwargs)

class W65C22(elm.Ic):
    def __init__(self, *args, **kwargs):
        pins = [
            elm.IcPin(name='VSS',  side='l', pin='1',  slot='20/20'),
            elm.IcPin(name='PA0',  side='l', pin='2',  slot='19/20'),
            elm.IcPin(name='PA1',  side='l', pin='3',  slot='18/20'),
            elm.IcPin(name='PA2',  side='l', pin='4',  slot='17/20'),
            elm.IcPin(name='PA3',  side='l', pin='5',  slot='16/20'),
            elm.IcPin(name='PA4',  side='l', pin='6',  slot='15/20'),
            elm.IcPin(name='PA5',  side='l', pin='7',  slot='14/20'),
            elm.IcPin(name='PA6',  side='l', pin='8',  slot='13/20'),
            elm.IcPin(name='PA7',  side='l', pin='9',  slot='12/20'),
            elm.IcPin(name='PB0',  side='l', pin='10', slot='11/20'),
            elm.IcPin(name='PB1',  side='l', pin='11', slot='10/20'),
            elm.IcPin(name='PB2',  side='l', pin='12', slot='9/20'),
            elm.IcPin(name='PB3',  side='l', pin='13', slot='8/20'),
            elm.IcPin(name='PB4',  side='l', pin='14', slot='7/20'),
            elm.IcPin(name='PB5',  side='l', pin='15', slot='6/20'),
            elm.IcPin(name='PB6',  side='l', pin='16', slot='5/20'),
            elm.IcPin(name='PB7',  side='l', pin='17', slot='4/20'),
            elm.IcPin(name='CB1',  side='l', pin='18', slot='3/20'),
            elm.IcPin(name='CB2',  side='l', pin='19', slot='2/20'),
            elm.IcPin(name='VDD',  side='l', pin='20', slot='1/20'),
            elm.IcPin(name='CA1',  side='r', pin='40', slot='20/20'),
            elm.IcPin(name='CA2',  side='r', pin='39', slot='19/20'),
            elm.IcPin(name='RS0',  side='r', pin='38', slot='18/20'),
            elm.IcPin(name='RS1',  side='r', pin='37', slot='17/20'),
            elm.IcPin(name='RS2',  side='r', pin='36', slot='16/20'),
            elm.IcPin(name='RS3',  side='r', pin='35', slot='15/20'),
            elm.IcPin(name='RESB', side='r', pin='34', slot='14/20'),
            elm.IcPin(name='D0',   side='r', pin='33', slot='13/20'),
            elm.IcPin(name='D1',   side='r', pin='32', slot='12/20'),
            elm.IcPin(name='D2',   side='r', pin='31', slot='11/20'),
            elm.IcPin(name='D3',   side='r', pin='30', slot='10/20'),
            elm.IcPin(name='D4',   side='r', pin='29', slot='9/20'),
            elm.IcPin(name='D5',   side='r', pin='28', slot='8/20'),
            elm.IcPin(name='D6',   side='r', pin='27', slot='7/20'),
            elm.IcPin(name='D7',   side='r', pin='26', slot='6/20'),
            elm.IcPin(name='PHI2', side='r', pin='25', slot='5/20'),
            elm.IcPin(name='CS1',  side='r', pin='24', slot='4/20'),
            elm.IcPin(name='CS2B', side='r', pin='23', slot='3/20'),
            elm.IcPin(name='RWB',  side='r', pin='22', slot='2/20'),
            elm.IcPin(name='IRQB', side='r', pin='21', slot='1/20'),
        ]
        super().__init__(pins=pins, w=5, plblofst=.05, botlabel='W65C22 VIA', **kwargs)


def draw(**opts):
    """opts: arduino, data, buttons, lcd, ram"""
    d = schemdraw.Drawing(fontsize=11, inches_per_unit=.6)
    use_arduino = opts.get('arduino', True)
    use_data    = opts.get('data', True)
    use_buttons = opts.get('buttons', True)
    use_lcd     = opts.get('lcd', False)
    use_ram     = opts.get('ram', False)

    CPU = d.add(W65C02S())

    # ── CPU Ström ──
    d.add(elm.Line('l', at=CPU.VDD, l=d.unit)); d.add(elm.Vdd(label='+5V')); d.add(elm.Dot())
    d.push(); d.add(elm.Capacitor('d', label='C1', l=1.5)); d.add(elm.Ground()); d.pop()
    d.add(elm.Line('l', l=1.0)); d.add(elm.Capacitor('d', label='C2')); d.add(elm.Ground())
    d.add(elm.Line('r', at=CPU.VSS, l=d.unit)); d.add(elm.Ground())

    # ── Pull-ups (BE kopplas separat till GND — aktiv låg!) ──
    offsets = {'RDY': 0.5, 'IRQB': 1.0, 'NMIB': 1.5, 'SOB': 0.5}
    for i, pn in enumerate(['RDY','IRQB','NMIB','SOB'], 1):
        pin = CPU.anchors[pn]; side = 'r' if pin[0] > 0 else 'l'
        d.add(elm.Line(side, at=pin, l=offsets[pn])); d.add(elm.Dot())
        d.push(); d.add(elm.Line('u', l=1.5)); d.add(elm.Resistor('u', label=f'R{i}'))
        d.add(elm.Vdd(label='+5V')); d.pop()

    # ── Oanvända CPU-pinnar ──
    for pn, side in [('VPB','l'),('MLB','l'),('PHI1O','l'),('NC','r'),('PHI2O','r')]:
        d.add(elm.Line(side, at=CPU.anchors[pn], l=0.4))

    # ── BE till GND (aktiv låg — måste vara LÅG för att bussarna ska drivas) ──
    d.add(elm.Line('r', at=CPU.BE, l=0.5)); d.add(elm.Ground())

    if not use_arduino:
        # ── Arduino (ej ansluten i detta steg) ──
        d.add(ArduinoMega2560(at=[CPU.D0[0]+10, CPU.D0[1]]))
        # ── Klock-LED för steg 1 (till höger om SOB, vertikalt nedåt) ──
        led_x = CPU.SOB[0] + 1.0  # Till höger om SOB pull-up
        d.add(elm.Line('r', at=CPU.PHI2, l=led_x - CPU.PHI2[0], color='blue'))
        d.add(elm.Dot(at=(led_x, CPU.PHI2[1]), color='blue'))
        d.add(elm.Resistor('u', label='R6'))
        d.add(elm.LED('u', label='LED1'))
        d.add(elm.Line('r', l=0.3))
        d.add(elm.Ground())
        return d
        return d

    # ── Arduino ──
    Mega = d.add(ArduinoMega2560(at=[CPU.D0[0]+10, CPU.D0[1]]))
    data_base = CPU.D0[0] + 3.5
    ctrl_base = data_base - 0.8
    addr_base = data_base + 2.0

    # ── Kontrollbuss ──
    for cp, mp in [('PHI2','D2_PHI2'), ('RESB','D4_RESET')]:
        cp_pin = getattr(CPU, cp); mp_pin = getattr(Mega, mp)
        d.add(elm.Line('r', at=cp_pin, l=ctrl_base - cp_pin[0], color='blue'))
        if mp_pin[1] != cp_pin[1]:
            d.add(elm.Line('u' if mp_pin[1] > cp_pin[1] else 'd', at=(ctrl_base, cp_pin[1]),
                           l=abs(mp_pin[1] - cp_pin[1]), color='blue'))
        d.add(elm.Line('r', at=(ctrl_base, mp_pin[1]), l=mp_pin[0] - ctrl_base, color='blue'))

    # ── Klock-LED (till höger om SOB, vertikalt nedåt) ──
    led_x = CPU.SOB[0] + 1.0
    d.add(elm.Dot(at=(ctrl_base, CPU.PHI2[1]), color='blue'))
    d.push()
    d.add(elm.Line('l', l=ctrl_base - led_x, color='blue'))
    d.add(elm.Dot(at=(led_x, CPU.PHI2[1]), color='blue'))
    d.add(elm.Resistor('u', label='R6'))
    d.add(elm.LED('u', label='LED1'))
    d.add(elm.Line('r', l=0.3))
    d.add(elm.Ground())
    d.pop()

    if use_data:
        d.add(elm.Line('r', at=CPU.RWB, l=ctrl_base - CPU.RWB[0], color='blue'))
        d.add(elm.Line('u' if Mega.D3_RW[1] > CPU.RWB[1] else 'd', l=abs(Mega.D3_RW[1] - CPU.RWB[1]), color='blue'))
        d.add(elm.Line('r', tox=Mega.D3_RW[0], color='blue'))

    # ── Adressbuss ──
    for i in range(12, 16):
        cp = getattr(CPU, f'A{i}'); mp = getattr(Mega, f'A{i-8}_A{i-8}')
        x_tunnel = addr_base + (i-12) * 0.15
        d.add(elm.Line('r', at=cp, l=x_tunnel - cp[0], color='orange'))
        if mp[1] != cp[1]:
            d.add(elm.Line('u' if mp[1] > cp[1] else 'd', at=(x_tunnel, cp[1]), l=abs(mp[1] - cp[1]), color='orange'))
        d.add(elm.Line('r', at=(x_tunnel, mp[1]), l=mp[0] - x_tunnel, color='orange'))

    for i in range(12):
        cp = getattr(CPU, f'A{i}')
        mp = getattr(Mega, f'A{8+i}_A{8+i}') if i < 8 else getattr(Mega, f'A{i-8}_A{i-8}')
        dist_left = 0.5 + i * 0.15
        loop_y = CPU.A11[1] - 1.5 - i * 0.15
        x_tunnel = addr_base + (i + 4) * 0.15
        d.add(elm.Line('l', at=cp, l=dist_left, color='orange'))
        d.add(elm.Line('d', at=(cp[0] - dist_left, cp[1]), l=cp[1] - loop_y, color='orange'))
        d.add(elm.Line('r', at=(cp[0] - dist_left, loop_y), l=x_tunnel - (cp[0] - dist_left), color='orange'))
        d.add(elm.Line('u', at=(x_tunnel, loop_y), l=mp[1] - loop_y, color='orange'))
        d.add(elm.Line('r', at=(x_tunnel, mp[1]), l=mp[0] - x_tunnel, color='orange'))

    # ── Databuss ──
    if use_data:
        for i in range(8):
            cp = getattr(CPU, f'D{i}')
            mp = getattr(Mega, f'D{22+i}_D{i}')
            d.add(elm.Line('r', at=cp, l=data_base + i*0.2 - cp[0], color='green'))
            if mp[1] != cp[1]:
                d.add(elm.Line('u' if mp[1] > cp[1] else 'd', at=(data_base + i*0.2, cp[1]),
                               l=abs(mp[1] - cp[1]), color='green'))
            d.add(elm.Line('r', at=(data_base + i*0.2, mp[1]), l=mp[0] - data_base - i*0.2, color='green'))

    # ── Knappar ──
    if use_buttons:
        d.add(elm.Line('r', at=Mega.D11_BTN1, l=1)); d.add(elm.Button('r', label='BTN1')); d.add(elm.Ground())
        d.add(elm.Line('r', at=Mega.D12_BTN2, l=1)); d.add(elm.Button('r', label='BTN2')); d.add(elm.Ground())

    d.add(elm.Line('r', at=Mega.V5, l=1)); d.add(elm.Vdd(label='+5V'))
    d.add(elm.Line('r', at=Mega.GND_pin, l=1)); d.add(elm.Ground())

    # ── LCD ──
    if use_lcd:
        LCD = d.add(elm.Header(rows=4, shownumber=False,
                               at=[Mega.SDA[0]+5, Mega.SDA[1]], anchor='p1',
                               label='LCD1602 I2C', pinsright=['SDA','SCL','VCC','GND']))
        d.add(elm.Line('r', at=Mega.SDA, l=2)); d.add(elm.Line('d', toy=LCD.p1[1])); d.add(elm.Line('r', tox=LCD.p1[0]))
        d.add(elm.Line('r', at=Mega.SCL, l=2.5)); d.add(elm.Line('u', toy=LCD.p2[1])); d.add(elm.Line('r', tox=LCD.p2[0]))
        d.add(elm.Line('r', at=LCD.p3, l=d.unit/2)); d.add(elm.Vdd(label='+5V'))
        d.add(elm.Line('r', at=LCD.p4, l=d.unit/2)); d.add(elm.Ground())

    # ── RAM ──
    if use_ram:
        # ── 62256 SRAM (kommenterad tills vidare) ──
        # ram_y = CPU.A11[1] - 12
        # RAM = d.add(SRAM62256(at=[0, ram_y]))
        # d.add(elm.Line('r', at=RAM.VCC, l=1.0)); d.add(elm.Vdd(label='+5V'))
        # d.add(elm.Line('l', at=RAM.GND, l=1.0)); d.add(elm.Ground())
        # d.add(elm.Line('r', at=RAM.VCC, l=0.5))
        # d.add(elm.Capacitor('d', label='C4', toy=RAM.GND[1]))
        # d.add(elm.Ground())

        # 74HC00 + 74HC04 behöver en y-referens — använd samma som innan
        ram_y = CPU.A11[1] - 12

        # ── 74HC00 adressavkodning (A14, A15 → RAM /CE) ──
        dec_x = 4.0
        dec_y = ram_y + 4.0
        n1 = d.add(logic.Nand(at=(dec_x, dec_y + 1.5), scale=0.8, label='1/4'))
        n2 = d.add(logic.Nand(at=(dec_x, dec_y - 1.5), scale=0.8, label='1/4'))
        n3 = d.add(logic.Nand(at=(dec_x + 4, dec_y), scale=0.8, label='1/4'))
        # NAND1: NOT A14 (both inputs = A14)
        d.add(elm.Line('l', at=n1.in1, l=2.0, lftlabel='A14'))
        d.add(elm.Line('l', at=n1.in2, l=2.0))
        # NAND2: NOT A15
        d.add(elm.Line('l', at=n2.in1, l=2.0, lftlabel='A15'))
        d.add(elm.Line('l', at=n2.in2, l=2.0))
        # NAND3: A14 AND A15 → RAM /CE
        d.add(elm.Line('r', at=n1.out, to=n3.in1))
        d.add(elm.Line('r', at=n2.out, to=n3.in2))
        d.add(elm.Line('r', at=n3.out, l=1.0, rgtlabel='→ RAM /CE'))

        # RAM kontroll (kommenterad)
        # d.add(elm.Line('r', at=RAM.WEB, l=1.5, rgtlabel='← R/W'))
        # d.add(elm.Line('l', at=RAM.OEB, l=3, lftlabel='← 74HC04'))
        # d.add(elm.Line('l', at=RAM.OEB, l=5.0))
        # 74HC04 (kommenterad)
        # d.add(elm.Line('u', l=0.5))
        # d.add(elm.Ic(
        #     pins=[elm.IcPin(name='A', side='l', slot='1/1'), elm.IcPin(name='Y', side='r', slot='1/1')],
        #     w=2, plblofst=.05, botlabel='74HC04'))
        # d.add(elm.Line('l', l=1.5, lftlabel='← R/W'))
        # d.add(elm.Line('r', l=1.5, rgtlabel='→ /OE'))

        # ── W65C22 VIA (under Arduino Mega) ──
        via_x = Mega.D4_RESET[0]
        via_y = ram_y + 2.5
        VIA = d.add(W65C22(at=[via_x, via_y]))

        # Bussanslutningar — förläng stammar ned till VIA
        # Databuss D0-D7: VIA-pinnar på höger sida, anslut till stammar
        for i in range(8):
            x_tr = data_base + i * 0.2
            dp = VIA.anchors[f'D{i}']
            # Dra stam ned till VIA-nivå om den inte redan går så långt
            d.add(elm.Line('l', at=dp, tox=x_tr, color='green'))
            d.add(elm.Dot(at=(x_tr, dp[1]), color='green'))

        # Adressbuss A0-A3 (RS0-RS3 på VIA:s högra sida)
        for i in range(4):
            x_tr = addr_base + i * 0.22
            rp = VIA.anchors[f'RS{i}']
            d.add(elm.Line('l', at=rp, tox=x_tr, color='orange'))
            d.add(elm.Dot(at=(x_tr, rp[1]), color='orange'))

        # Kontrollsignaler (PHI2, RWB, RESB — alla på VIA:s högra sida)
        d.add(elm.Line('l', at=VIA.PHI2, tox=ctrl_base, color='blue'))
        d.add(elm.Dot(at=(ctrl_base, VIA.PHI2[1]), color='blue'))
        d.add(elm.Line('l', at=VIA.RWB,  tox=ctrl_base, color='blue'))
        d.add(elm.Dot(at=(ctrl_base, VIA.RWB[1]), color='blue'))
        d.add(elm.Line('l', at=VIA.RESB, tox=ctrl_base, color='blue'))
        d.add(elm.Dot(at=(ctrl_base, VIA.RESB[1]), color='blue'))

        # Ström (VDD/VSS på vänster sida)
        d.add(elm.Line('l', at=VIA.VDD, l=1.0)); d.add(elm.Vdd(label='+5V'))
        d.add(elm.Line('l', at=VIA.VSS, l=1.0)); d.add(elm.Ground())

        # Chip select (CS1, CS2B — höger sida)
        d.add(elm.Line('l', at=VIA.CS1,  l=1.5, lftlabel='A14 →'))
        d.add(elm.Line('l', at=VIA.CS2B, l=1.5, lftlabel='A15 →'))

    return d


draw(arduino=False).save(f'{OUT}/step1.svg');                     print('Steg 1: OK')
draw(arduino=True, data=False, buttons=False).save(f'{OUT}/step2.svg'); print('Steg 2: OK')
draw(arduino=True, data=True,  buttons=False).save(f'{OUT}/step3.svg'); print('Steg 3: OK')
draw(arduino=True, data=True,  buttons=True,  lcd=False).save(f'{OUT}/step4.svg'); print('Steg 4: OK')
draw(arduino=True, data=True,  buttons=True,  lcd=True ).save(f'{OUT}/step5.svg'); print('Steg 5: OK')
draw(arduino=True, data=True,  buttons=True,  lcd=True, ram=True).save(f'{OUT}/step7.svg'); print('Steg 7: OK')
draw(arduino=True, data=True,  buttons=True,  lcd=True, ram=True).save(f'{OUT}/sheet1-overview.svg'); print('Sheet1: OK')
