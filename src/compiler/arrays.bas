print "Array stuff"
goto mylabel
label mylabel
rem Allocate 4 bytes for S.
alloc buffer = 4
rem {} = 32-bit array access, @@ = 16-bit array access, [] = 8-bit array access
let B=100
rem 122233 = Decimal 0xFFF2FFF = Hex, 0b10101111 = Binary
let {buffer} = 0xA10F2CD+0x100ABC+(0b1010111001 + 0b01)
let X = [buffer]
print "(uint8_t)S[0] and (uint32_t)S[0] = "; X, {buffer}
let S = 0
end
