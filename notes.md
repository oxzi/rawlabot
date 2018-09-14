## Scan Profile

- short-range: penetrative scanning (?)
- sensor: high-res, slower capture rate
- sensor narrow: low-res, faster capture rate


## Get Action

- raw signals: each signal (*i, j*) as reflected pulse; sent from antenna *i*,
  reflected from target and recieved in antenna *j*.


## Arena

Def.: Configurable space and image resolution in which the Walabot works.

- shor-range
    - arena size should be similar to the Walabot
    - specified by Cartesian coordinates (*x, y, z* axes)
- sensor/sensor narrow
    - "arena is usually much larger [sic]" - whatever that means
    - specified by Spherical coordinates (radial distance *r*,
      angles *theta/phi*)

### Formula

- x = r * sin theta
- y = r * cos theta * sin phi
- z = r * cos theta * cos phi


## Sources

- [Walabot API, Imaging Features](https://api.walabot.com/_features.html)
