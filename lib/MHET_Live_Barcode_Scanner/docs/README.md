# mhet-live-barcode-scanner
Library to use the MH-ET LIVE Barcode scanner module

## Configuration

### Structure

Any configuration command consists of four parts:

- command prefix (`~M`)
- option code
- value
- command suffix (`.`)

The option code and the value are hex encoded decimal values which are converted to a string and transmitted over a serial connection.

The module returns different values:

- `[ACK]`: Command and value were set successfully
- `[NAK]`: Command was valid but value was invalid
- `[ENQ]`: Command was invalid

Any parameter can be queried by using the _command prefix_ `~Q` instead of `~M`. The response is one of

- `<option code><value>[ACK]`: Query was successful and option and value are returned
- `[ENQ]`: Command was invalid

The scanner can also be controlled in manual mode and scanning can be triggered by sending `~T.` as code. The response is one of

- `T[ACK]`: Manual scan successful
- `T[NAK]`: Manual scan not successful

#### Option codes:

##### Special codes

| Code   | Description |
|:------:|-------------|
| `A5F0` | Special code to access save and restore functions, only documented with value `1B2C` to fully reset device, `506A` to save user settings, `8F37` to restore user defaults and `D201` to delete user defaults; using this command with other values are **untested** and could **brick** the device |
| `0091` | Setup code, enters and leaves configuration mode |
| `00F5` | Sets baud rate |
| `F672` | Read module information |
| `FA50` | Read all setup codes |
| `0051` | Not documented, seems to fix missing output |

##### Operation codes

| Code   | Description |
|:------:|-------------|
| `0001` | Enable or disable all 1D codes |
| `0002` | Enable or disable all 2D codes |
| `0021` | System working mode |
| `0022` | Sleep settings for continuous mode |
| `0023` | Sensitivity of sensing in induction mode |
| `0024` | Horizontal mirroring |
| `0025` | Vertical mirroring |
| `0026` | Night vision for induction mode |
| `0073` | Trigger mode answer **(1)** |
| `0086` | Fill light in trigger mode |
| `0092` | Line breaks on output |
| `0093` | TAB option **(1)** |
| `0094` | Automatically add prefix |
| `0095` | Automatically add suffix |
| `009B` | Code ID mode **(1)** |
| `009C` | Convert output to uppercase or lowercase |
| `00B0` | Same code identification delay |
| `00B1` | Single reading time |
| `00B2` | Reading interval |
| `00EA` | Tone mode for bootup |
| `00EB` | Tone mode for decode |
| `00EC` | Tone mode for configure |
| `00FA` | Volume for bootup tone |
| `00FB` | Volume for decode tone |
| `00FC` | Volume for configuration tone |
| `0103` | Fill light in continuous mode |
| `0105` | Aiming light **(1)** |
| `010A` | Startup indicator |
| `010B` | Decoding indicator |
| `010C` | Successful configuration indicator |
| `0126` | Fill light in induction mode |
| `0145` | Enable or disable Codebar codes |
| `0150` | Enable or disable Code128 codes |
| `0160` | Enable or disable Code39 codes |
| `0185` | Enable or disable "Interleaved 2 of 5" codes |
| `01B0` | Enable or disable QR codes |
| `01BA` | Enable or disable UPC/EAN/JAN codes |
| `01C0` | Enable or disable Code93 codes |
| `01E5` | Enable or disable "Industrial 2 of 5" codes |
| `0200` | Enable or disable "Matrix 2 of 5" codes |
| `1000` | Enable or disable Code11 codes |
| `1100` | Enable or disable MSI codes |
| `1200` | Enable or disable GS1 Databar codes |
| `1300` | Enable or disable ISBN codes |
| `1326` | Enable or disable ISSN codes |
| `1353` | Enable or disable CODE32 codes |

**(1)**: I really dont know what this does and I need to test it. **Please use with caution!**

All option codes with specified values can be found in the file [MHET_Live_Barcode_Scanner.h](./include/MHET_Live_Barcode_Scanner.h).

### Time calculation

The official documentation has preprinted timings for several delays and interrupts. They are calculated by the basis of `100ms`, converted to hex and injected to the string.

The `AutomaticModeSingleReadingTimeDelay` uses the option code `00B1`. The timing delay for five seconds is the hex converted value of `50` (as 50 times 100ms is 5s). The complete command would be `~M00B10032.`.

It should be possible to inject a custom timing value with this procedure to the scanner module, but this needs testing.

### Troubleshooting

If there are errors after resetting the device, you can issue the following commands:

```
~M00910001.             # Enter setup mode
~M00510000.             # NOT DOCUMENTED
~MA5F0506A.             # Save user settings
~M00910000.             # Exit user mode
```