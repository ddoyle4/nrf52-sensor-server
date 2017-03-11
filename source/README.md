### TODO
- Remove the individual 'local stages' from each of the sensor stores
and make them share just one. When something is flushed just write to
here instead. Otherwise is a waste of space.
