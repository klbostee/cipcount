from glob import glob

def mapper(data):
    import cipcount_impl
    return cipcount_impl.mapper(data)

def reducer(data):
    import cipcount_impl
    return cipcount_impl.reducer(data)

def runner(job):
    job.additer(mapper, reducer, combiner=reducer)

def starter(prog):
    egg = glob("dist/cipcount*egg")[0]
    prog.addopt("libegg", egg)

if __name__ == "__main__":
    from dumbo import main
    main(runner, starter)
