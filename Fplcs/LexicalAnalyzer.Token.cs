﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Fplcs
{
    public partial class LexicalAnalyzer<TTokenType>
    {
        public class Token
        {
            public readonly TTokenType Type;
            public Token(TTokenType type)
            {
                Type = type;
            }
            public override bool Equals(object obj)
            {
                var other = obj as Token;
                return other != null && other.Type.Equals(Type);
            }
            public override int GetHashCode()
            {
                return Type.GetHashCode();
            }
        }

        public delegate Token TokenCallback(string input, int pos, int length);

        class TokenDefinition
        {
            public readonly string Regex;
            public readonly RegexOptions RegexOptions;
            public readonly TokenCallback TokenCallback;
            public TokenDefinition(string regex, RegexOptions regexOptions, TokenCallback tokenCallback)
            {
                Regex = regex;
                RegexOptions = regexOptions;
                TokenCallback = tokenCallback;
            }
        }

        Dictionary<TTokenType, TokenDefinition> tokenDefinitions = new Dictionary<TTokenType, TokenDefinition>();

        public void AddTokenDefinition(TTokenType type, string regex, RegexOptions regexOptions = RegexOptions.None, TokenCallback tokenCallback = null)
        {
            if (string.IsNullOrEmpty(regex)) throw new Exception("Regex cannot be null or empty");
            if (finalized) unfinalize();
            if (tokenDefinitions.ContainsKey(type)) throw new Exception("A previous definition for the given token type already exists");
            tokenDefinitions.Add(type, new TokenDefinition(regex, regexOptions, tokenCallback));
        }
    }
}
