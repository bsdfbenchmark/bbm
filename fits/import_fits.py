"""! Import a fit file into a dictionary where the key is the material name

    @param filename = filename of the fit file
    @param m = bbm module name
    @returns a dictionary where the key is the name of the material and the value is the corresponding bbm bsdf

    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.py}
    import bbm_floatRGB as bbm
    fits = import_fitst("../fits/ngan_ward.txt", bbm)
    str( fits['alum-bronze'] )
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

"""
def import_fits(filename, m):

    # create dict
    globalDict = {}
    if m != None:
        for d in dir(m):
            globalDict[d] = getattr(m, d)
    else:
        globalDict = None

    # create dict of results
    fits = {}
    
    fitFile = open(filename, "r")
    fitLines = fitFile.readlines()
    fitFile.close()
    
    for line in fitLines:
        if  not line.startswith('#'):
            split = line.find('=')
            key = line[0:split-1].strip()
            val = line[split+1:-1].strip()

            fits[key] = eval(val, globalDict)

    # Done
    return fits
