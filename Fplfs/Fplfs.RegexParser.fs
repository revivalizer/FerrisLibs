﻿module Fplfs.RegexParser
    type CharacterClass =
        | AnyCharacter
        | CharacterSet of bool * char list

    type RegexAstNode =
        | CharAstNode of char
        | SequenceAstNode of RegexAstNode list
        | ZeroOrMoreAstNode of RegexAstNode
        | OneOrMoreAstNode of RegexAstNode
        | ZeroOrOneAstNode of RegexAstNode
        | OrAstNode of RegexAstNode * RegexAstNode
        | CharacterClassAstNode of CharacterClass

    let parseRegex (s : string) =
        let rec parseExpression parenLevel pos =
            let finishExpression = function
                | [] -> failwith "Expression cannot be empty"
                | [x] -> x
                | x -> SequenceAstNode (List.rev x)

            let rec parseChars acc pos =
                if pos >= s.Length then
                    if parenLevel > 0 then failwith "Expected ')'"
                    (acc, pos)
                else
                    let pos' = pos + 1

                    let handleModifierChar c f = function
                        | [] -> failwith (sprintf "'%c' found with no preceding expression" c)
                        | head :: tail -> parseChars (f head :: tail) pos'

                    let parseEscapeSequence pos =
                        if pos >= s.Length then failwith "Unexpected end of string"
                        match s.[pos] with
                        | '\\' -> '\\'
                        | 't' -> '\t'
                        | 'r' -> '\r'
                        | 'n' -> '\n'
                        | c ->
                            // TODO: This is structured this way for character class specialization later
                            match c with
                            | '(' | ')' | '|' | '*' | '+' | '?' | '$' | '[' | '.' -> c
                            | _ -> failwith "Invalid character escape sequence"

                    match s.[pos] with
                    | '*' -> handleModifierChar '*' ZeroOrMoreAstNode acc
                    | '+' -> handleModifierChar '+' OneOrMoreAstNode acc
                    | '?' -> handleModifierChar '?' ZeroOrOneAstNode acc
                    | '|' ->
                        let left = finishExpression acc
                        let right, pos' = parseExpression parenLevel pos'
                        ([OrAstNode (left, right)], pos')
                    | '(' ->
                        let expr, pos' = parseExpression (parenLevel + 1) pos'
                        parseChars (expr :: acc) pos'
                    | ')' ->
                        if parenLevel <= 0 then failwith "')' with no preceeding '('"
                        (acc, pos')
                    | '\\' -> parseChars (CharAstNode (parseEscapeSequence pos') :: acc) (pos' + 1)
                    | '.' -> parseChars (CharacterClassAstNode AnyCharacter :: acc) pos'
                    | '[' ->
                        let rec parseClassChars startPos isInverse acc pos =
                            if pos >= s.Length then failwith "Unexpected end of string" // TODO: Test
                            let pos' = pos + 1
                            match s.[pos] with
                            | ']' -> (isInverse, acc, pos')
                            | c -> parseClassChars startPos isInverse (c :: acc) pos'
                        let isInverse, chars, pos' = parseClassChars pos' false [] pos'
                        parseChars (CharacterClassAstNode (CharacterSet (isInverse, List.rev chars)) :: acc) pos'
                    | x -> parseChars (CharAstNode x :: acc) pos'

            let contents, pos' = parseChars [] pos
            (finishExpression contents, pos')

        fst (parseExpression 0 0)
