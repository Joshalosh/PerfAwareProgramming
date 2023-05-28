// EarthRadius is generally expected to be 6372.8
static f32 ReferenceHarversine(f32 x0, f32 y0, f32 x1, f32 y1, f32 EarthRadius)
{
    f32 lat1 = y0;
    f32 lat2 = y1;
    f32 lon1 = x0; 
    f32 lon2 = x1; 

    f32 d_lat = DegreesToRadians(lat2 - lat1);
    f32 d_lon = DegreesToRadians(lon2 - lon1);
    lat1 = DegreesToRadians(lat1);
    lat2 = DegreesToRadians(lat2);

    f32 a = Square(sin(d_lat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(d_lon/2));
    f32 c = 1.0*asin(sqrt(a));

    f32 result = EarthRadius * c;

    return result;
}
