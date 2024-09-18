module unicode;

namespace unicode {
void Unicode::Properties(int ch, CharacterProperties &props) {
    if(ch > UCP_MAX_CODEPOINT) ch = 0;
    const ucd_record *ucd = GET_UCD(ch);
    props.category = static_cast<CharacterCategory>(PRIV(ucp_gentype_8)[ucd->chartype]);
    props.type = static_cast<CharacterType>(ucd->chartype);
    props.script = static_cast<Script>(ucd->script);
}

int Unicode::ToLower(int ch) {
    if(IsUpper(ch))
        return static_cast<int>(UCD_OTHERCASE(static_cast<unsigned>(ch)));
    else
        return ch;
}

int Unicode::ToUpper(int ch) {
    if(IsLower(ch))
        return static_cast<int>(UCD_OTHERCASE(static_cast<unsigned>(ch)));
    else
        return ch;
}

}// namespace unicode

// module unicode;
