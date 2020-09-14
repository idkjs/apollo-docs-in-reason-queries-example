module ApolloQueryResult = ApolloClient.Types.ApolloQueryResult;
module QueryResult = ApolloClient.Types.QueryResult;

module GetDogs = [%graphql {|
    {dogs {id breed}}
|}];

[%graphql
  {|
  query GetDogPhoto($breed: String!) {
    dog(breed: $breed) {
      id
      displayImage
    }
  }
|}
];
module DogPhoto = {
  [@react.component]
  let make = (~breed) => {
    let variables = GetDogPhoto.makeVariables(~breed, ());
    let queryResult =
      GetDogPhoto.use(
        ~notifyOnNetworkStatusChange=true,
        // ~pollInterval=1500,
        variables,
      );

    switch (queryResult) {
    | {networkStatus: Refetch} => <p> "Refetching!"->React.string </p>
    | {loading: true} => React.null
    | {error: Some(error)} =>
      {
        "Error!: " ++ error.message;
      }
      ->React.string
    | {data: Some({dog: Some({displayImage})})} =>
      <div>
        <div>
          {displayImage
           ->Belt.Option.map(displayImage =>
               <img
                 src=displayImage
                 style={ReactDOM.Style.make(~height="100", ~width="100", ())}
               />
             )
           ->Belt.Option.getWithDefault(React.null)}
        </div>
        <button
          onClick={_ =>
            queryResult->QueryResult.refetch(
              // you have to pass the variables back in or wont do anything
              GetDogPhoto.serializeVariables(variables),
            )
            |> ignore
          }>
          "Refetch!"->React.string
        </button>
      </div>
    | _ => React.null
    };
  };
};
module Dogs = {
  [@react.component]
  let make = (~onDogSelected) => {
    let result = GetDogs.use();

    switch (result) {
    | {loading: true} => "Loading..."->React.string
    | {error: Some(error)} => React.string("Error!: " ++ error.message)

    | {data: Some({dogs})} =>
      let dogs =
        dogs->Belt.Option.getWithDefault([||])->Belt.Array.keepMap(d => d);

      <select name="dog" onChange=onDogSelected>
        {dogs
         ->Belt.Array.map(dog =>
             <option key={dog.id} value={dog.breed}>
               {dog.breed |> React.string}
             </option>
           )
         ->React.array}
      </select>;

    | _ => React.null
    };
  };
};
let client =
  ApolloClient.(
    make(
      ~cache=Cache.InMemoryCache.make(),
      ~connectToDevTools=true,
      ~link=
        Link.HttpLink.make(~uri=_ => "https://71z1g.sse.codesandbox.io/", ()),
      (),
    )
  );
[@react.component]
let make = () => {
  let (selectedDog, setSelectedDog) = React.useState(() => None);

  let onDogSelected = event => {
    ReactEvent.Synthetic.preventDefault(event);
    setSelectedDog(_ => event->ReactEvent.Form.target##value);
  };
  <ApolloClient.React.ApolloProvider client>
    <div>
      <h2> {React.string("Building Query components " ++ {js|🚀|js})} </h2>
      {switch (selectedDog) {
       | Some(selectedDog) => <DogPhoto breed=selectedDog />
       | None => React.null
       }}
      <Dogs onDogSelected />
    </div>
  </ApolloClient.React.ApolloProvider>;
};
