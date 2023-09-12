# Is this the shortest solution? I hope it is
def gen_quiz(qpool, *quindexes, altcodes="ABCDEF", quiz=None):
    for i in quindexes:
        try: (quiz := ([] if quiz is None else quiz)).append((qpool[i][0], [": ".join(answer) for answer in list(zip(altcodes, qpool[i][1]))]))
        except Exception as e: print("Ignoring index %s - %s" % (i, e))
    return quiz
