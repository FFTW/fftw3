file(READ ${CPYFILE} copyrgt_content)
file(READ ${PREFILE} prelude_content)
file(READ ${FMAFILE} topfile_content)
file(READ ${STDFILE} botfile_content)

string(TIMESTAMP _cur_time "%Y-%m-%d %H:%MZ" UTC)
string(REPLACE "@DATE@" "${_cur_time}" _stamped_prelude ${prelude_content})

string(APPEND fused_content "${copyrgt_content}\n")
string(APPEND fused_content "${_stamped_prelude}\n\n\n")
string(APPEND fused_content "#ifdef HAVE_FMA\n\n")
string(APPEND fused_content "${topfile_content}")
string(APPEND fused_content "\n#else /* HAVE_FMA */\n\n\n")
string(APPEND fused_content "${botfile_content}")
string(APPEND fused_content "\n#endif /* HAVE_FMA */\n")

file(WRITE ${OUTFILE} "${fused_content}")
