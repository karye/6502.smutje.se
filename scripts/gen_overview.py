#!/usr/bin/env python3
"""Generate step 1–5, 7 schematics — parameterstyrd."""
import schemdraw
import schemdraw.elements as elm
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
            *[elm.IcPin(name=f'A{8+i}_A{i}', pin=f'A{8+i}', side='l', slot=f'{23-i}/40') for i in range(8)],
            *[elm.IcPin(name=f'A{i}_A{8+i}', pin=f'A{i}',   side='l', slot=f'{15-i}/40') for i in range(8)],
            elm.IcPin(name='D5_BTN1', pin='5',   side='r', slot='36/40'),
            elm.IcPin(name='D6_BTN2', pin='6',   side='r', slot='32/40'),
            elm.IcPin(name='SDA',     pin='SDA', side='r', slot='24/40'),
            elm.IcPin(name='SCL',     pin='SCL', side='r', slot='20/40'),
            elm.IcPin(name='V5',      pin='5V',  side='r', slot='8/40'),
            elm.IcPin(name='GND_pin',     pin='GND', side='r', slot='4/40'),
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
    d.push(); d.add(elm.Capacitor('d', label='100n (C1)', l=1.5)); d.add(elm.Ground()); d.pop()
    d.add(elm.Line('l', l=1.0)); d.add(elm.Capacitor('d', label='10uF (C2)')); d.add(elm.Ground())
    d.add(elm.Line('r', at=CPU.VSS, l=d.unit)); d.add(elm.Ground())

    # ── Pull-ups ──
    offsets = {'RDY': 0.5, 'IRQB': 1.0, 'NMIB': 1.5, 'BE': 0.8, 'SOB': 1.8}
    for i, pn in enumerate(['RDY','IRQB','NMIB','BE','SOB'], 1):
        pin = CPU.anchors[pn]; side = 'r' if pin[0] > 0 else 'l'
        d.add(elm.Line(side, at=pin, l=offsets[pn])); d.add(elm.Dot())
        d.push(); d.add(elm.Line('u', l=1.5)); d.add(elm.Resistor('u', label=f'3.3K (R{i})'))
        d.add(elm.Vdd(label='+5V')); d.pop()

    # ── Oanvända CPU-pinnar ──
    for pn, side in [('VPB','l'),('MLB','l'),('PHI1O','l'),('NC','r'),('PHI2O','r')]:
        d.add(elm.Line(side, at=CPU.anchors[pn], l=0.4))

    if not use_arduino:
        # ── Arduino (ej ansluten i detta steg) ──
        d.add(ArduinoMega2560(at=[CPU.D0[0]+10, CPU.D0[1]]))
        # ── Klock-LED för steg 1 (direkt från PHI2) ──
        clk_x = CPU.PHI2[0] + 1.5
        d.add(elm.Line('r', at=CPU.PHI2, l=clk_x - CPU.PHI2[0], color='blue'))
        d.add(elm.Dot(at=(clk_x, CPU.PHI2[1]), color='blue'))
        d.add(elm.Line('u', at=(clk_x, CPU.PHI2[1]), l=1.8, color='blue'))
        d.add(elm.Resistor('r', label='220 (R6)'))
        d.add(elm.LED('r', label='LED1'))
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

    # ── Klock-LED (från reset-linjen på kontrollbussen) ──
    rst_y = Mega.D4_RESET[1]
    d.add(elm.Dot(at=(ctrl_base, rst_y), color='blue'))
    d.push()
    d.add(elm.Line('l', l=3.5, color='blue'))
    d.add(elm.Resistor('l', label='220 (R6)'))
    d.add(elm.LED('l', label='LED1'))
    d.add(elm.Line('d', l=1.5))
    d.add(elm.Ground())
    d.pop()

    if use_data:
        d.add(elm.Line('r', at=CPU.RWB, l=ctrl_base - CPU.RWB[0], color='blue'))
        d.add(elm.Line('u' if Mega.D3_RW[1] > CPU.RWB[1] else 'd', l=abs(Mega.D3_RW[1] - CPU.RWB[1]), color='blue'))
        d.add(elm.Line('r', tox=Mega.D3_RW[0], color='blue'))

    # ── Adressbuss ──
    for i in range(12, 16):
        cp = getattr(CPU, f'A{i}'); mp = getattr(Mega, f'A{i-8}_A{i}')
        x_tunnel = addr_base + (i-12) * 0.15
        d.add(elm.Line('r', at=cp, l=x_tunnel - cp[0], color='orange'))
        if mp[1] != cp[1]:
            d.add(elm.Line('u' if mp[1] > cp[1] else 'd', at=(x_tunnel, cp[1]), l=abs(mp[1] - cp[1]), color='orange'))
        d.add(elm.Line('r', at=(x_tunnel, mp[1]), l=mp[0] - x_tunnel, color='orange'))

    for i in range(12):
        cp = getattr(CPU, f'A{i}')
        mp = getattr(Mega, f'A{8+i}_A{i}') if i < 8 else getattr(Mega, f'A{i-8}_A{i}')
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
        d.add(elm.Line('r', at=Mega.D5_BTN1, l=1)); d.add(elm.Button('r', label='BTN1')); d.add(elm.Ground())
        d.add(elm.Line('r', at=Mega.D6_BTN2, l=1)); d.add(elm.Button('r', label='BTN2')); d.add(elm.Ground())

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
        ram_y = CPU.A11[1] - 12
        RAM = d.add(SRAM62256(at=[0, ram_y]))
        d.add(elm.Line('r', at=RAM.VCC, l=1.0)); d.add(elm.Vdd(label='+5V'))
        d.add(elm.Line('l', at=RAM.GND, l=1.0)); d.add(elm.Ground())
        d.add(elm.Line('r', at=RAM.VCC, l=0.5))
        d.add(elm.Capacitor('d', label='100n', toy=RAM.GND[1]))
        d.add(elm.Ground())
        d.add(elm.Line('r', at=RAM.CEB, l=2, rgtlabel='← A15'))
        d.add(elm.Line('r', at=RAM.WEB, l=1.5, rgtlabel='← R/W'))
        d.add(elm.Line('l', at=RAM.OEB, l=3, lftlabel='← 74HC04'))
        d.add(elm.Line('l', at=RAM.OEB, l=5.0))
        d.add(elm.Line('u', l=0.5))
        d.add(elm.Ic(
            pins=[elm.IcPin(name='A', side='l', slot='1/1'), elm.IcPin(name='Y', side='r', slot='1/1')],
            w=2, plblofst=.05, botlabel='74HC04'))
        d.add(elm.Line('l', l=1.5, lftlabel='← R/W'))
        d.add(elm.Line('r', l=1.5, rgtlabel='→ /OE'))

    return d


draw(arduino=False).save(f'{OUT}/step1.svg');                     print('Steg 1: OK')
draw(arduino=True, data=False, buttons=False).save(f'{OUT}/step2.svg'); print('Steg 2: OK')
draw(arduino=True, data=True,  buttons=False).save(f'{OUT}/step3.svg'); print('Steg 3: OK')
draw(arduino=True, data=True,  buttons=True,  lcd=False).save(f'{OUT}/step4.svg'); print('Steg 4: OK')
draw(arduino=True, data=True,  buttons=True,  lcd=True ).save(f'{OUT}/step5.svg'); print('Steg 5: OK')
draw(arduino=True, data=True,  buttons=True,  lcd=True, ram=True).save(f'{OUT}/step7.svg'); print('Steg 7: OK')
draw(arduino=True, data=True,  buttons=True,  lcd=True, ram=True).save(f'{OUT}/sheet1-overview.svg'); print('Sheet1: OK')
